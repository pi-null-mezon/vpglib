#-------------------------------------------------
#
# Project created by QtCreator 2015-12-10T12:47:25
#
#-------------------------------------------------

TARGET = test_Device
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

include($${PWD}/../lib/opencv.pri)
include($${PWD}/../lib/exportvpg.pri)
