// MinerWatch  - A watchguard for your miner.
// Copyright © 2017  Stian "Wooly" Bogevik
//
// This file is part of MinerWatch.
//
// MinerWatch is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3 of the License.

// MinerWatch is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MinerWatch.  If not, see <http://www.gnu.org/licenses/>.

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timeSinceMinerStarted=QDateTime::currentDateTime().addYears(1);
    ui->buttonGroup->setId(ui->radioButton_pool_nanozec, 1);
    ui->buttonGroup->setId(ui->radioButton_pool_nanoeth, 2);
    ui->buttonGroup->setId(ui->radioButton_ethpool, 3);

    LoadSettings();        
    ui->statusBar->hide();
    minerprocess = new QProcess(this);    
    setMinerBat(ui->lineEdit_miner_loc->text());


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
    ui->buttonGroup->setExclusive(true);
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
        setMinerBat(ui->lineEdit_miner_loc->text());
    }
    catch (...)
    {

    }
}

void MainWindow::hashrateUpdated(double hashrate)
{
    ui->label_hashrate->setText(QString::number(hashrate)+"");
    if (lastReportedHashrate != hashrate)
    {
        //qDebug() << "ny hashrate mottatt...:" << QDateTime::currentDateTime().toString("[hh:mm:ss]") << hashrate;

        qint64 time = QDateTime::currentDateTime().toMSecsSinceEpoch();
        hashSeries->append(time, hashrate);
        hashScatterSeries->append(time, hashrate);
        hashScatterSeries->setPointLabelsVisible(true);
        hashScatterSeries->setPointLabelsFormat("@yPoint");
        hashBottomSeries->append(time, 0);

        if (ui->checkBox_autoshowlast->isChecked())
        {
            ui->dateTimeEdit_hash_from->blockSignals(true);
            ui->dateTimeEdit_hash_to->blockSignals(true);

            QDateTime from, to;
            to   = QDateTime::currentDateTime();
            from = to.addSecs(ui->spinBox_hourinterval->value() * -60 * 60);

            hashChart->axisX()->setMin(from);
            hashChart->axisX()->setMax(to);

            int tickcount = (from.secsTo(to) / 60) / 60;

            qobject_cast<QDateTimeAxis*>(hashChart->axisX())->setTickCount(tickcount+1);

            ui->dateTimeEdit_hash_from->setDateTime(from);
            ui->dateTimeEdit_hash_to->setDateTime(to);

            ui->dateTimeEdit_hash_from->blockSignals(false);
            ui->dateTimeEdit_hash_to->blockSignals(false);
        }

        if (hashrate > maxReportedHashrate)
        {
            maxReportedHashrate = hashrate;
            hashChart->axisY()->setMax(hashrate + (hashrate * 0.05));
        }
        if (hashrate < minReportedHashrate)
        {
                minReportedHashrate = hashrate;                
                hashChart->axisY()->setMin(hashrate);
        }
        lastReportedHashrate=hashrate;
    }

    if (hashrate <= ui->spinBox->value() && QDateTime::currentDateTime().toMSecsSinceEpoch() >= timeSinceMinerStarted.addSecs(60*10).toMSecsSinceEpoch() && minerprocess->state() == QProcess::Running)
    {
        //hashrate is 0 -> restart miner.
        manual_restart=1;
        minerprocess->close();
        minerprocess->waitForFinished(5000);
        delayedStart.start();
        timeSinceMinerStarted = QDateTime::currentDateTime();
        QString log = "<b>NanoPool reported a hashrate of: "+QString::number(hashrate)+" for your worker. Killing miner</b>";
        api->makeLog(log);
        ui->textBrowser->append(QDateTime::currentDateTime().toString(datetimestring) + log);
        manual_restart=0;        
    }
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
    ui->spinBox->setValue(settings.value("sol_limit").toInt());
    ui->checkBox_autoshowlast->setChecked(settings.value("autoshowlast_checkbox", 0).toBool());
    ui->spinBox_hourinterval->setValue(settings.value("autoshowlast_hours", 6).toInt());

    api = new nanopool_api(ui->lineEdit_account->text(), ui->lineEdit_worker->text(), this);
    connect(api, SIGNAL(hashrateUpdate(double)), this, SLOT(hashrateUpdated(double)));

    int i = settings.value("pool", 0).toInt();
    if (ui->buttonGroup->button(i) != 0x0)
        ui->buttonGroup->button(i)->setChecked(true);

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
    settings.setValue("pool", ui->buttonGroup->checkedId());
    settings.setValue("autoshowlast_checkbox", ui->checkBox_autoshowlast->isChecked());
    settings.setValue("autoshowlast_hours", ui->spinBox_hourinterval->value());
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
        delayedStart.stop();
        manual_restart=1;
        minerprocess->close();
        minerprocess->waitForFinished(-1);

        manual_restart=0;
        ui->pushButton_2->setText("Start");

        QString log = "Miner manually stopped.";
        api->makeLog(log);
        ui->textBrowser->append(QDateTime::currentDateTime().toString(datetimestring) + log);
    }
}

void MainWindow::minerprocess_stateChanged(QProcess::ProcessState state)
{
    if (state == QProcess::NotRunning)
    {
        qDebug() << "error:" << minerprocess->errorString();
        minerprocess->terminate();
        minerprocess->close();
        ui->label_status->setText("NOT MINING");
        ui->label_status->setStyleSheet("*{ color: red;}");
        if (!manual_restart)
        {
           delayedStart.start();           
           QString log = "Miner exited. Restarting miner "+QString::number(delayedStart.interval() / 1000)+" seconds from now.";
           ui->textBrowser->append(QDateTime::currentDateTime().toString(datetimestring) + log);

           api->makeLog(log);
        }
    }
    if (state == QProcess::Running)
    {
        timeSinceMinerStarted = QDateTime::currentDateTime();
        QString log = "Miner started.";
        api->makeLog(log);
        ui->textBrowser->append(QDateTime::currentDateTime().toString(datetimestring) + log);
        ui->label_status->setText("MINING");
        ui->label_status->setStyleSheet("*{ color: green;}");
    }
}

void MainWindow::on_HashChart_DateChanged()
{
    ui->checkBox_autoshowlast->setChecked(false);
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
        qDebug() << minerprocess->program() << minerprocess->arguments();
        minerprocess->start();
        ui->pushButton_2->setText("Stop");
    }
}

void MainWindow::setMinerBat(QString file)
{
    QFile bat(file);
    QString prg;
    QStringList args;
    QDir dir;
    QFileInfo info(ui->lineEdit_miner_loc->text());
    if (bat.exists())
    {
        if (bat.size() > 100000)
        {
            qDebug() << "is excecutable...";

             minerprocess->setWorkingDirectory(dir.toNativeSeparators(info.absoluteDir().absolutePath()));
             minerprocess->setProgram(minerprocess->workingDirectory()+QDir::separator()+bat.fileName());
             return;
        }
       if (bat.open(QIODevice::ReadOnly))
       {
           QByteArray data = bat.readAll();
           QList<QByteArray> lst = data.split(' ');

           prg = lst.at(0);
           for (int i=1; i<lst.count(); i++)
           {
               args << lst.at(i);
           }
           minerprocess->setWorkingDirectory(dir.toNativeSeparators(info.absoluteDir().absolutePath()));
           minerprocess->setProgram(minerprocess->workingDirectory()+QDir::separator()+prg);
           minerprocess->setArguments(args);
       }
    }
}

void MainWindow::on_radioButton_pool_nanozec_toggled(bool checked)
{
    if (checked)
    {
        api->setPoolAndCoin("nanopool", "zec");
    }
}

void MainWindow::on_radioButton_pool_nanoeth_toggled(bool checked)
{
    if (checked)
    {
        api->setPoolAndCoin("nanopool", "eth");
    }
}

void MainWindow::on_radioButton_ethpool_toggled(bool checked)
{
    if (checked)
    {
        api->setPoolAndCoin("ethpool", "eth");
    }
}

void MainWindow::on_checkBox_toggled(bool checked)
{
        hashScatterSeries->setPointLabelsVisible(!checked);
}

void MainWindow::on_checkBox_2_toggled(bool checked)
{
    hashScatterSeries->setVisible(!checked);
}

void MainWindow::on_pushButton_3_pressed()
{
    QApplication::clipboard()->setText("16YhAJTpepry3UNezXmHxGGASYKo31csao");
    QMessageBox::information(this, "Thank you!", "Donation Bitcoin address \"16YhAJTpepry3UNezXmHxGGASYKo31csao\" copied to clipboard.", QMessageBox::Ok);
}

void MainWindow::on_pushButton_4_pressed()
{
    QApplication::clipboard()->setText("t1ZdNd3DB6JDznisbNhHxuEK5kyEF5G5avV");
    QMessageBox::information(this, "Thank you!", "Donation ZCash address \"t1ZdNd3DB6JDznisbNhHxuEK5kyEF5G5avV\" copied to clipboard.", QMessageBox::Ok);
}


void MainWindow::on_actionOpen_log_file_triggered()
{
    QDesktopServices::openUrl(QUrl(QApplication::applicationDirPath() + "/minerwatch.log"));
}

void MainWindow::on_actionMinerWatch_triggered()
{
    AboutDialog diag;
    diag.exec();
}

void MainWindow::on_pushButton_eth_pressed()
{
    QApplication::clipboard()->setText("0x1B74929CaEB99cF17FD603F734ccE9Df9b752Bd7");
    QMessageBox::information(this, "Thank you!", "Donation Ethereum address \"0x1B74929CaEB99cF17FD603F734ccE9Df9b752Bd7\" copied to clipboard.", QMessageBox::Ok);

}
