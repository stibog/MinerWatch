#-------------------------------------------------
#
# Project created by QtCreator 2017-04-29T10:35:37
#
#-------------------------------------------------

QT       += core gui network charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MinerWatch
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    nanopool_api.cpp

HEADERS  += mainwindow.h \
    nanopool_api.h

FORMS    += mainwindow.ui

RC_FILE = minerwatch.rc

CONFIG += c++11
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
