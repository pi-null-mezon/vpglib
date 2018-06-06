QT += gui websockets multimedia

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = VPGService
VERSION = 1.0.0.0

DEFINES += APP_NAME=\\\"$${TARGET}\\\" \
           APP_VERSION=\\\"$${VERSION}\\\" \
           APP_DESIGNER=\\\"Alex_A._Taranov\\\"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES +=  main.cpp \
            vpgservice.cpp

HEADERS +=  vpgservice.h

#----------------------------------------------------VPGLIB
DEFINES += VPG_BUILD_FROM_SOURCE

SOURCES +=  $${PWD}/../../../Library/src/faceprocessor.cpp \
            $${PWD}/../../../Library/src/hrvprocessor.cpp \
            $${PWD}/../../../Library/src/peakdetector.cpp \
            $${PWD}/../../../Library/src/pulseprocessor.cpp \

INCLUDEPATH += $${PWD}/../../../Library/include

include($${PWD}/../../../Shared/opencv.pri)
include($${PWD}/../../../Shared/openmp.pri)

#-------------------------------------------------qtservice
include($${PWD}/../qtservice/src/qtservice.pri)

include($${PWD}/../VPGServer/vpgserver.pri)

#CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT
