#------------------------------------------------------------------------
#
# MinerWatch  - A watchguard for your miner.
# Copyright © 2017  Stian "Wooly" Bogevik
#
# This file is part of MinerWatch.
#
# MinerWatch is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# MinerWatch is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with MinerWatch.  If not, see <http://www.gnu.org/licenses/>.
#
#------------------------------------------------------------------------

QT       += core gui network charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MinerWatch
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    nanopool_api.cpp \
    aboutdialog.cpp \
    miner_api.cpp

HEADERS  += mainwindow.h \
    nanopool_api.h \
    aboutdialog.h \
    miner_api.h

FORMS    += mainwindow.ui \
    aboutdialog.ui

RC_FILE = minerwatch.rc

CONFIG += c++11
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
