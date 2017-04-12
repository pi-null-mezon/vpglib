#-------------------------------------------------
#
# Project created by QtCreator 2015-12-10T12:47:25
#
#-------------------------------------------------

CONFIG -= qt

TARGET = hrvanalyser
VERSION = 1.0.0.0

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

include($${PWD}/../../shared/vpglib.pri)

#CONFIG += designbuild

designbuild {
    message(Design build mode selected)
    DEFINES += CASCADE_FILENAME=\\\"C:/Programming/3rdParties/opencv310/sources/data/haarcascades/\\\"
} else {
    message(Deploy build mode selected)
}

