CONFIG -= qt
CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = vpgtracker
VERSION = 1.0.2.0

DEFINES += APP_NAME=\\\"$${TARGET}\\\" \
           APP_VERSION=\\\"$${VERSION}\\\" \
           APP_DESIGNER=\\\"Alex_A._Taranov\\\"

SOURCES += main.cpp \
           facetracker.cpp

HEADERS += facetracker.h

include($${PWD}/../../Shared/dlib.pri) 
include($${PWD}/../../Shared/vpglib.pri)
