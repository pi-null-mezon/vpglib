SOURCES += $${PWD}/qvideosource.cpp \
           $${PWD}/qvideolocker.cpp \
           $${PWD}/qfacetracker.cpp \
           $${PWD}/facetracker.cpp \
           $${PWD}/qvpgserver.cpp

HEADERS += $${PWD}/qvideosource.h \
           $${PWD}/qvideolocker.h \
           $${PWD}/qfacetracker.h \
           $${PWD}/facetracker.h \
           $${PWD}/qvpgserver.h

INCLUDEPATH += $${PWD} \
               $${PWD}/../../vpgtracker

include($${PWD}/../../../Shared/opencv.pri)
include($${PWD}/../../../Shared/dlib.pri)

