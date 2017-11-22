CONFIG -= qt
CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = vpgtracker
VERSION = 1.0.0.1

DEFINES += APP_NAME=\\\"$${TARGET}\\\" \
           APP_VERSION=\\\"$${VERSION}\\\" \
           APP_DESIGNER=\\\"Alex_A._Taranov\\\"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp \
           facetracker.cpp

HEADERS += facetracker.h

include($${PWD}/dlib.pri)
include($${PWD}/opencv.pri)
include($${PWD}/../../Shared/vpglib.pri)
