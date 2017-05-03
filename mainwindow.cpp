#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    LoadSettings();
    api = new nanopool_api(ui->lineEdit_account->text(), ui->lineEdit_worker->text(), this);
    connect(api, SIGNAL(hashrateUpdate(double)), this, SLOT(hashrateUpdated(double)));
    ui->statusBar->hide();
    minerprocess = new QProcess(this);
    QDir dir;
    QFileInfo info(ui->lineEdit_miner_loc->text());
    minerprocess->setWorkingDirectory(dir.toNativeSeparators(info.absoluteDir().absolutePath()));
    //minerprocess->setEnvironment(QProcess::systemEnvironment());

    QFile bat(ui->lineEdit_miner_loc->text());
    QString prg;
    QStringList args;
    if (bat.exists())
    {
       if (bat.open(QIODevice::ReadOnly))
       {
           QByteArray data = bat.readAll();
           qDebug() << data;
           QList<QByteArray> lst = data.split(' ');
           prg = lst.at(0);
           for (int i=1; i<lst.count(); i++)
           {
               args << lst.at(i);
           }
           minerprocess->setProgram(minerprocess->workingDirectory()+QDir::separator()+prg);
           minerprocess->setArguments(args);
       }
    }


    connect(minerprocess, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(minerprocess_stateChanged(QProcess::ProcessState)));
    connect(ui->lineEdit_account, SIGNAL(textEdited(QString)), api, SLOT(setAccountName(QString)));
    connect(ui->lineEdit_worker, SIGNAL(textEdited(QString)), api, SLOT(setWorkerName(QString)));
    delayedStart.setInterval(500);
    delayedStart.setSingleShot(true);
    connect(&delayedStart, SIGNAL(timeout()), this, SLOT(StartMining()));

    hashChart = new QChart();
    hashSeries = new QLineSeries(this);    
    hashScatterSeries = new QScatterSeries(this);
    hashBottomSeries = new QLineSeries(this);


    hashScatterSeries->setColor(QColor().black());    
    hashScatterSeries->setMarkerSize(7);
    hashScatterSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);

    hashChart->addSeries(hashSeries);
    hashChart->addSeries(hashBottomSeries);
    hashChart->addSeries(hashScatterSeries);
    hashChart->legend()->hide();
    hashChart->setTitle("Hashrate Chart");

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("dddd<br>hh:mm");

    axisX->setTitleText("Time");

    qint64 exmin = QDateTime::currentDateTime().currentDateTime().toString("mm").toInt();
    exmin =60*exmin;
    exmin = (exmin - (exmin *2));
    QDateTime from, to;
    from = QDateTime::currentDateTime().addSecs(exmin);
    to = QDateTime::currentDateTime().addDays(1).addSecs(exmin);
    axisX->setMin(from);
    ui->dateTimeEdit_hash_from->setDateTime(from);
    ui->dateTimeEdit_hash_to->setDateTime(to);
    axisX->setMax(to);

    axisX->setTickCount(25);
    hashChart->addAxis(axisX, Qt::AlignBottom);
    hashSeries->attachAxis(axisX);
    hashBottomSeries->attachAxis(axisX);
    hashScatterSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%i");
    axisY->setTitleText("Hashrate");
    hashChart->addAxis(axisY, Qt::AlignLeft);
    hashSeries->attachAxis(axisY);
    hashBottomSeries->attachAxis(axisY);
    hashScatterSeries->attachAxis(axisY);

    hashChartView = new QChartView(hashChart);
    hashChartView->setRenderHint(QPainter::Antialiasing);

    ui->verticalLayout_groupboxchart->insertWidget(0, hashChartView);
    /*QHBoxLayout *lay = new QHBoxLayout();
    lay->addWidget(hashChartView);
    lay->addWidget(ui->horizontalLayout_2);
    ui->groupBox_chart->setLayout(lay);*/

    connect(ui->dateTimeEdit_hash_to, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_HashChart_DateChanged()));
    connect(ui->dateTimeEdit_hash_from, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(on_HashChart_DateChanged()));

}

MainWindow::~MainWindow()
{

    manual_restart=1;
    minerprocess->close();
  //  minerprocess->terminate();
//    minerprocess->kill();
    minerprocess->waitForFinished(-1);
    SaveSettings();
    delete hashChart;
    delete hashSeries;
    delete hashScatterSeries;
    delete hashBottomSeries;
    delete hashChartView;
    delete ui;
    delete api;
    minerprocess->deleteLater();    
}

void MainWindow::on_pushButton_pressed()
{
    try {
        ui->lineEdit_miner_loc->setText(QFileDialog::getOpenFileName(this,"Please locate your miner .exe or .bat", ui->lineEdit_miner_loc->text(), "Executable (*.bat *.exe)"));
        //minerprocess->setProgram(ui->lineEdit_miner_loc->text());
        QDir dir;
        minerprocess->setWorkingDirectory(dir.toNativeSeparators(ui->lineEdit_miner_loc->text()));

        QFile bat(ui->lineEdit_miner_loc->text());
        QString prg;
        QStringList args;
        if (bat.exists())
        {
           if (bat.open(QIODevice::ReadOnly))
           {
               QList<QByteArray> lst = bat.readAll().split(' ');
               prg = lst.at(0);
               for (int i=1; i<lst.count(); i++)
               {
                   args << lst.at(i);
               }
               minerprocess->setProgram(minerprocess->workingDirectory()+QDir::separator()+prg);
               minerprocess->setArguments(args);
           }
        }
    }
    catch (...)
    {

    }
}

void MainWindow::hashrateUpdated(double hashrate)
{
    ui->label_hashrate->setText(QString::number(hashrate)+" SOL");
    if (lastReportedHashrate != hashrate)
    {
        //qDebug() << "ny hashrate mottatt...:" << QDateTime::currentDateTime().toString("[hh:mm:ss]") << hashrate;

        qint64 time = QDateTime::currentDateTime().toMSecsSinceEpoch();
        hashSeries->append(time, hashrate);
        hashScatterSeries->append(time, hashrate);
        hashScatterSeries->setPointLabelsVisible(true);
        hashScatterSeries->setPointLabelsFormat("@yPoint");
        hashBottomSeries->append(time, 0);
        if (hashrate > maxReportedHashrate)
        {
            maxReportedHashrate = hashrate;
            hashChart->axisY()->setMax(hashrate+15);
        }
        qDebug() << "if" << hashrate << "<" << minReportedHashrate;
        if (hashrate < minReportedHashrate)
        {
            qDebug() << "true...";
                minReportedHashrate = hashrate;                
                hashChart->axisY()->setMin(hashrate);
        }
        lastReportedHashrate=hashrate;
    }

    if (hashrate <= ui->spinBox->value() && QDateTime::currentDateTime().toMSecsSinceEpoch() >= timeSinceMinerStarted.addSecs(60*10).toMSecsSinceEpoch())
    {
        if (timesLow <= 7)
        {
            timesLow++;
            return;
        }

        //hashrate is 0 -> restart miner.
        manual_restart=1;
        minerprocess->close();
        minerprocess->waitForFinished(5000);

        delayedStart.start();
        timeSinceMinerStarted = QDateTime::currentDateTime().addSecs(60*10);
        QString log = "<b>NanoPool reported "+QString::number(hashrate)+" sol hashrate for your worker 6x times in a row. Killing miner</b>";
        api->makeLog(log);
        ui->textBrowser->append(log);
        manual_restart=0;        
    }
    timesLow=0;
}

void MainWindow::LoadSettings()
{
    QSettings settings("SB_Software", "MinerWatch");
    settings.beginGroup("MinerWatch_settings");
    QRect windowGeometry = settings.value("mainwindow_position").toRect();
    if (windowGeometry.isValid())
        this->setGeometry(windowGeometry);
    ui->lineEdit_miner_loc->setText(settings.value("miner.exe","").toString());
    ui->lineEdit_worker->setText(settings.value("worker","").toString());
    ui->lineEdit_account->setText(settings.value("account","").toString());
    ui->spinBox->setValue(settings.value("sol_limit", 0).toInt());
    settings.endGroup();
}

void MainWindow::SaveSettings()
{
    QSettings settings("SB_Software", "MinerWatch");
    settings.beginGroup("MinerWatch_settings");
    settings.setValue("mainwindow_position",this->geometry());
    settings.setValue("miner.exe",ui->lineEdit_miner_loc->text());
    settings.setValue("worker", ui->lineEdit_worker->text());
    settings.setValue("account", ui->lineEdit_account->text());
    settings.setValue("sol_limit", ui->spinBox->value());
    settings.endGroup();
}

void MainWindow::on_pushButton_2_pressed()
{
    if (ui->pushButton_2->text() == "Start")
    {
        StartMining();
    }
    else
    {
        manual_restart=1;
        minerprocess->close();
        minerprocess->waitForFinished(-1);
        manual_restart=0;
        ui->pushButton_2->setText("Start");

        QString log = "Miner manually stopped.";
        api->makeLog(log);
        ui->textBrowser->append(log);
    }
}

void MainWindow::minerprocess_stateChanged(QProcess::ProcessState state)
{
    if (state == QProcess::NotRunning)
    {

        minerprocess->terminate();
        minerprocess->close();
        ui->label_status->setText("NOT MINING");
        ui->label_status->setStyleSheet("*{ color: red;}");
        if (!manual_restart)
        {
           delayedStart.start();           
           QString log = "Miner exited. Restarting miner 0.5 seconds from now.";
           ui->textBrowser->append(log);

           api->makeLog(log);
        }
    }
    if (state == QProcess::Running)
    {
        timeSinceMinerStarted = QDateTime::currentDateTime();
        QString log = "Miner started.";
        api->makeLog(log);
        ui->textBrowser->append(log);
        ui->label_status->setText("MINING");
        ui->label_status->setStyleSheet("*{ color: green;}");
    }
}

void MainWindow::on_HashChart_DateChanged()
{
    QDateTime from, to;
    from = ui->dateTimeEdit_hash_from->dateTime();
    to   = ui->dateTimeEdit_hash_to->dateTime();
    hashChart->axisX()->setMin(from);
    hashChart->axisX()->setMax(to);

    int tickcount = (from.secsTo(to) / 60) / 60;

    qobject_cast<QDateTimeAxis*>(hashChart->axisX())->setTickCount(tickcount+1);


}

void MainWindow::on_actionQt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::StartMining()
{
    QFileInfo bat(ui->lineEdit_miner_loc->text());
    if (bat.exists())
    {
        minerprocess->start();
        ui->pushButton_2->setText("Stop");
    }
}
