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

#ifndef NANOPOOL_API_H
#define NANOPOOL_API_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>
#include <QtCharts>
#include <QtCharts>
using namespace QtCharts;

class nanopool_api : public QObject
{
    Q_OBJECT
public:
    explicit nanopool_api(QString account, QString worker, QObject *parent = 0);
    ~nanopool_api();
    void makeLog(QString data);
    void setPoolAndCoin(QString p, QString c);

signals:
    void hashrateUpdate(double hashrate);

public slots:
    void setAccountName(QString account);
    void setWorkerName(QString worker);
    void getCurrentHashRate();

private slots:
    void netReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *netManager;
    void sendRequest(QString req);
    QString account_name, worker_name;
    QTimer *hashRateTimer;
    QString pool="nanopool";
    QString coin="zec";
    QString email="";
};

#endif // NANOPOOL_API_H
