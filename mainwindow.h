#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QSettings>
#include <QProcess>
#include "nanopool_api.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_pressed();
    void hashrateUpdated(double hashrate);
    void on_pushButton_2_pressed();
    void minerprocess_stateChanged(QProcess::ProcessState state);

    void on_HashChart_DateChanged();

    void on_actionQt_triggered();
    void StartMining();

private:
    Ui::MainWindow *ui;
    void LoadSettings();
    void SaveSettings();
    nanopool_api *api;
    double lastReportedHashrate=0, maxReportedHashrate=0, minReportedHashrate=9999999999999999.0;
    QProcess *minerprocess;
    bool manual_restart=0;
    QString datetimestring = "[dd.MM.yyyy - hh:mm:ss]";
    QTimer delayedStart;

    QChart *hashChart;
    QLineSeries *hashSeries, *hashBottomSeries;
    QScatterSeries *hashScatterSeries;
    QChartView *hashChartView;


    int timesLow =0;
    QDateTime timeSinceMinerStarted;

};

#endif // MAINWINDOW_H
