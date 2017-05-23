#ifndef MINER_API_H
#define MINER_API_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

class miner_api : public QObject
{
    Q_OBJECT
public:
    explicit miner_api(QObject *parent = 0);
    ~miner_api();
    void setMinerType(QString miner);
    void startstop(bool start);

signals:

    void statusupdate(QString update);

public slots:
    void queryMiner();

private slots:
    void readyRead();

private:
    QString active_miner;
    QTcpSocket *socket;
    QTimer *updateTimer;
};

#endif // MINER_API_H
