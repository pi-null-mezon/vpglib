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

include($${PWD}/../../Shared/vpglib.pri)

linux {
	DEFINES += TARGET_OS_LINUX
}

