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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QSettings>
#include <QProcess>
#include "nanopool_api.h"
#include "aboutdialog.h"

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
    void on_radioButton_pool_nanozec_toggled(bool checked);
    void on_radioButton_pool_nanoeth_toggled(bool checked);
    void on_radioButton_ethpool_toggled(bool checked);
    void on_checkBox_toggled(bool checked);
    void on_checkBox_2_toggled(bool checked);
    void on_pushButton_3_pressed();
    void on_pushButton_4_pressed();

    void on_actionOpen_log_file_triggered();

    void on_actionMinerWatch_triggered();

    void on_pushButton_eth_pressed();

    void on_radioButton_slushpool_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    void setMinerBat(QString file);
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


   // int timesLow =0;
    QDateTime timeSinceMinerStarted;

};

#endif // MAINWINDOW_H
