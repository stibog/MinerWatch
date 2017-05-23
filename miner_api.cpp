#include "miner_api.h"

miner_api::miner_api(QObject *parent) : QObject(parent)
{
    /*netManager = new QNetworkAccessManager(this);
    connect(netManager, SIGNAL(finished(QNetworkReply*)),
              this, SLOT(netReplyFinished(QNetworkReply*)));*/

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readChannelFinished()), this, SLOT(readyRead()));
    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000 * 2);
    updateTimer->setTimerType(Qt::VeryCoarseTimer);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(queryMiner()));

   // QTimer::singleShot(250, Qt::CoarseTimer, this, SLOT(queryMiner()));


}

miner_api::~miner_api()
{
    delete socket;
}

void miner_api::setMinerType(QString miner)
{
    if (!miner.isEmpty())
        active_miner = miner;
    else
        return;
    if (miner == "ewbf")
    {

    }

}

void miner_api::startstop(bool start)
{
    if (start)
        updateTimer->start();
    else
        updateTimer->stop();
}

void miner_api::queryMiner()
{
    if (active_miner == "ewbf")
    {
        socket->connectToHost("127.0.0.1", 42000);
        if (socket->waitForConnected(3000))
        {
            qDebug() << "successfulyl connected to miner api";

            socket->write("{\"id\":0, \"method\":\"getstat\"}\n");
            socket->flush();
        }
        return;
    }
}

void miner_api::readyRead()
{
    QByteArray data = socket->readAll();
    qDebug() << data;
    if (active_miner == "ewbf")
    {
        QJsonObject json = QJsonDocument::fromJson(data).object();

        QJsonArray array = json.value("result").toArray();

        QString status = "<table>";
        double totashr, totrshr, totspeed, totpwr;
        totashr = 0;
        totrshr = 0;
        totspeed= 0;
        totpwr  = 0;
        for (int i=0; i<array.count(); i++)
        {
            QJsonObject obj = array.at(i).toObject();
            status.append("<tr>");
            status.append("<td style=\"font-weight:600;\" width=\"50\">");
                status.append("GPU0");
            status.append("</td>");
            status.append("<td style=\"text-align: left;padding-left:20px;\">");
                status.append("Hashrate: ");
                double speed = obj.value("speed_sps").toDouble();
                totspeed += speed;
                status.append("<b>"+QString::number(speed)+ " Sol/s</b>");
            status.append("</td>");
            status.append("<td style=\"text-align: left;padding-left:20px;\">");
                status.append("    Temp: <b>");
                double temp = obj.value("temperature").toDouble();
                status.append(QString::number(temp)+"°C");
            status.append("</td>");
            status.append("<td style=\"text-align: left;padding-left:20px;\">");
                status.append("    Watt: <b>");
                double watt = obj.value("gpu_power_usage").toDouble();
                totpwr += watt;
                status.append(QString::number(watt));
            status.append("</td>");
            status.append("<td style=\"text-align: left;padding-left:20px;\">");
                status.append("    Sol/W: <b>");
                status.append(QString::number(obj.value("speed_sps").toDouble() / obj.value("gpu_power_usage").toDouble()));
            status.append("</td>");
            status.append("<td style=\"color: green;text-align: left;padding-left:20px;\">");
                status.append("    Accepted shares: <b>");
                double ashare = obj.value("accepted_shares").toDouble();
                totashr += ashare;
                status.append(QString::number(ashare));
            status.append("</td>");
            status.append("<td style=\"color: red;text-align: left;padding-left:20px;\">");
                status.append("    Rejected shares: <b>");
                double rshare = obj.value("rejected_shares").toDouble();
                if (rshare)
                    totrshr += rshare;
                status.append(QString::number(rshare));
            status.append("</td></tr>");
        }

        if (array.count() > 1)
        {
                status.append("<tr><td>&nbsp;</td></tr>");
                status.append("<tr style=\"border-top:1px;border-color: black;\">");
                status.append("<td style=\"font-weight:600;border-top: 1px;border-color: black;\" width=\"50\">");
                    status.append("<b>Total</b>");
                status.append("</td>");
                status.append("<td style=\"text-align: left;padding-left:20px;border-top:1px;border-color: black;\" colspan=2>");
                    status.append("Hashrate: <b>");
                    status.append(QString::number(totspeed)+" Sol/s");
                status.append("</td>");
                status.append("<td style=\"text-align: left;padding-left:20px;border-top:1px;border-color: black;\" colspan=2>");
                    status.append("    Watt: <b>");
                    status.append(QString::number(totpwr));
                status.append("</td>");
                status.append("<td style=\"color: green;text-align: left;padding-left:20px;border-top:1px;border-color: black;\">");
                    status.append("    Accepted shares: <b>");
                    status.append(QString::number(totashr));
                status.append("</td>");
                status.append("<td style=\"color: red;text-align: left;padding-left:20px;border-top:1px;border-color: black;\">");
                    status.append("    Rejected shares: <b>");
                    status.append(QString::number(totrshr));
                status.append("</td>");
        }
        status.append("<table>");
        emit statusupdate(status);
    }
}
