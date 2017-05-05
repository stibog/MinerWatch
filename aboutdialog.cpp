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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_pushButton_btc_pressed()
{
    QApplication::clipboard()->setText("16YhAJTpepry3UNezXmHxGGASYKo31csao");
    QMessageBox::information(this, "Thank you!", "Donation Bitcoin address \"16YhAJTpepry3UNezXmHxGGASYKo31csao\" copied to clipboard.", QMessageBox::Ok);
}

void AboutDialog::on_pushButton_zcash_pressed()
{
    QApplication::clipboard()->setText("t1ZdNd3DB6JDznisbNhHxuEK5kyEF5G5avV");
    QMessageBox::information(this, "Thank you!", "Donation ZCash address \"t1ZdNd3DB6JDznisbNhHxuEK5kyEF5G5avV\" copied to clipboard.", QMessageBox::Ok);
}

void AboutDialog::on_pushButton_eth_pressed()
{
    QApplication::clipboard()->setText("0x1B74929CaEB99cF17FD603F734ccE9Df9b752Bd7");
    QMessageBox::information(this, "Thank you!", "Donation Ethereum address \"0x1B74929CaEB99cF17FD603F734ccE9Df9b752Bd7\" copied to clipboard.", QMessageBox::Ok);

}
