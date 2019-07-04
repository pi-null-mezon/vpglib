#-------------------------------------------------
#
# Project created by QtCreator 2015-12-10T12:47:25
#
#-------------------------------------------------

TARGET = vpglight
VERSION = 1.0.0.5

DEFINES += APP_DESIGNER=\\\"Alex.A.Taranov\\\" \
           APP_NAME=\\\"$${TARGET}\\\" \
           APP_VERSION=\\\"$${VERSION}\\\"

CONFIG += console
CONFIG += c++11
CONFIG -= app_bundle
CONFIG -= qt

TEMPLATE = app

SOURCES += main.cpp

include($${PWD}/../../Shared/vpglib.pri)

CONFIG += designbuild

designbuild {
    message(Haarcascades will be searched in OPENCV_DATA_DIR)
    DEFINES += DESIGNBUILD
} else {
    message(Note, it is you responsibility to provide haarcascades to app)
    DEFINES += DEPLOYBUILD
}

linux {
	DEFINES += TARGET_OS_LINUX
}
