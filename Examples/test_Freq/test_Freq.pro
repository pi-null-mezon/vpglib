QT += core
QT -= gui

CONFIG += c++11

TARGET = test_Freq
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

include($${PWD}/../lib/opencv.pri)
include($${PWD}/../lib/exportvpg.pri)

