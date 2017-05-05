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
    QTimer::singleShot(250, Qt::CoarseTimer, this, SLOT(getCurrentHashRate()));
    hashRateTimer->start();
}

nanopool_api::~nanopool_api()
{
    netManager->deleteLater();
}

void nanopool_api::makeLog(QString data)
{
    QString logfile = QApplication::applicationDirPath() + "/minerwatch.log";
     QFile fil(logfile);
     if (fil.open(QIODevice::Append))
     {
         QTextStream stream(&fil);
         stream << QDateTime::currentDateTime().toString("[dd.MM.yyyy][hh:mm:ss] ") + data+"\r\n";
         fil.flush();
         fil.close();
     }
}

void nanopool_api::setPoolAndCoin(QString p, QString c)
{
    if (!p.isEmpty())
        this->pool = p;
    if (!c.isEmpty())
        this->coin = c;
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
    if (pool == "nanopool")
    {
        QString req = "hashrate/%1/%2";
        req = req.arg(account_name, worker_name);
        netManager->get(QNetworkRequest(QUrl("https://api.nanopool.org/v1/"+coin+"/"+req)));
    }
    else
        if (pool == "ethpool")
        {
            QString addr;
            if (account_name.mid(0,2) == "0x")
                addr = account_name.mid(2);
            else
                addr = account_name;
            netManager->get(QNetworkRequest(QUrl("http://ethpool.org/api/miner_new/"+addr)));
        }
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
     QJsonObject json = QJsonDocument::fromJson(data).object();
     if (pool == "nanopool")
     {
        if (json.contains("data"))
            emit hashrateUpdate(json.value("data").toDouble());
     }
     else
         if (pool == "ethpool")
         {
             json = json["workers"].toObject();

             if (json.keys().count() == 1)
             {
                 QString hashrate = json.value(json.keys().at(0)).toObject().value("hashrate").toString();
                 emit hashrateUpdate(hashrate.mid(0, hashrate.indexOf(" ")).toDouble());
             }
             else
             {
                 QString hashrate = json.value(worker_name).toObject().value("hashrate").toString();
                 emit hashrateUpdate(hashrate.mid(0, hashrate.indexOf(" ")).toDouble());
             }
         }
}
