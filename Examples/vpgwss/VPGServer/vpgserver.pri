SOURCES += $${PWD}/qvideosource.cpp \
           #$${PWD}/qvideolocker.cpp \
           $${PWD}/qfacetracker.cpp \
           $${PWD}/facetracker.cpp \
           $${PWD}/qvpgserver.cpp \
           $${PWD}/qvpgprocessor.cpp

HEADERS += $${PWD}/qvideosource.h \
           #$${PWD}/qvideolocker.h \
           $${PWD}/qfacetracker.h \
           $${PWD}/facetracker.h \
           $${PWD}/qvpgserver.h \
           $${PWD}/qvpgprocessor.h

INCLUDEPATH += $${PWD}

include($${PWD}/../../../Shared/dlib.pri)

