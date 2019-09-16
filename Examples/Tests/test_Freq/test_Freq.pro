
CONFIG += c++11
TARGET = test_Freq
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TEMPLATE = app

SOURCES += main.cpp

include($${PWD}/../../../Shared/vpglib.pri)

