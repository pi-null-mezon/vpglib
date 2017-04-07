#-------------------------------------------------
#
# Project created by QtCreator 2015-12-10T12:47:25
#
#-------------------------------------------------

TARGET = vpglight
VERSION = 1.0.0.3

DEFINES += APP_DESIGNER=\\\"Alex.A.Taranov\\\" \
           APP_NAME=\\\"$${TARGET}\\\" \
           APP_VERSION=\\\"$${VERSION}\\\"

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TEMPLATE = app

SOURCES += main.cpp

include($${PWD}/../../Shared/vpglib.pri)

CONFIG += designbuild

designbuild {
    message(Design build mode selected)
    DEFINES += DESIGNBUILD
} else {
    message(Deploy build mode selected)
    DEFINES += DEPLOYBUILD
}

