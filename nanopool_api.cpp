#include "nanopool_api.h"

nanopool_api::nanopool_api(QString account, QString worker, QObject *parent) : QObject(parent)
{
    this->account_name = account;
    this->worker_name  = worker;
    netManager = new QNetworkAccessManager(this);
    connect(netManager, SIGNAL(finished(QNetworkReply*)),
              this, SLOT(netReplyFinished(QNetworkReply*)));

    hashRateTimer = new QTimer(this);
    hashRateTimer->setInterval(1000 * 20);
    hashRateTimer->setTimerType(Qt::VeryCoarseTimer);
    connect(hashRateTimer, SIGNAL(timeout()), this, SLOT(getCurrentHashRate()));
    getCurrentHashRate();
    hashRateTimer->start();

}

nanopool_api::~nanopool_api()
{
    netManager->deleteLater();
}

void nanopool_api::makeLog(QString data)
{
    QString logfile = QApplication::applicationDirPath() + "/mottatt.log";
     QFile fil(logfile);
     if (fil.open(QIODevice::Append))
     {
         QTextStream stream(&fil);
         stream << QDateTime::currentDateTime().toString("[dd.MM.yyyy][hh:mm:ss]") + data+"\r\n";
         fil.flush();
         fil.close();
     }
}

void nanopool_api::setAccountName(QString account)
{
    this->account_name = account;
}

void nanopool_api::setWorkerName(QString worker)
{
    this->worker_name = worker;
}


void nanopool_api::getCurrentHashRate()
{
    QString req = "hashrate/%1/%2";
    req = req.arg(account_name, worker_name);
 //   qDebug() << "request = " << req;
    sendRequest(req);
}


void nanopool_api::netReplyFinished(QNetworkReply *reply)
{

     QByteArray data = reply->readAll();

     if (data.isEmpty())
     {
        makeLog("Received NULL reply. Clearing access Cache.");
        netManager->clearAccessCache();
        return;
     }
   //  makeLog("Received raw data: "+data);
     QJsonObject json = QJsonDocument::fromJson(data).object();
     if (json.contains("data"))
        emit hashrateUpdate(json.value("data").toDouble());
}
void nanopool_api::sendRequest(QString req)
{
    if (req.length())
        netManager->get(QNetworkRequest(QUrl("https://api.nanopool.org/v1/zec/"+req)));
}
