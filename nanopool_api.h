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
};

#endif // NANOPOOL_API_H
