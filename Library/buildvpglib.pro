TEMPLATE = lib

CONFIG(release, debug|release) {
    TARGET = vpg
} else {
    TARGET = vpgd
}

SOURCES += \
    $${PWD}/src/faceprocessor.cpp \
    $${PWD}/src/hrvprocessor.cpp \
    $${PWD}/src/peakdetector.cpp \
    $${PWD}/src/pulseprocessor.cpp

HEADERS += \
    $${PWD}/include/faceprocessor.h \
    $${PWD}/include/hrvprocessor.h \
    $${PWD}/include/peakdetector.h \
    $${PWD}/include/pulseprocessor.h \
    $${PWD}/include/vpg.h

INCLUDEPATH += $${PWD}/include

include($${PWD}/../Shared/opencv.pri)
include($${PWD}/../Shared/openmp.pri)
include($${PWD}/../Shared/opencl.pri)

#---------------------------------------------------------
DEFINES += DLL_BUILD_SETUP # is defined only if library build (for dll generation)
#---------------------------------------------------------

win32-msvc2010: COMPILER = vc10
win32-msvc2012: COMPILER = vc11
win32-msvc2013: COMPILER = vc12
win32-msvc2015: COMPILER = vc14
win32-g++:      COMPILER = mingw
win32:contains(QMAKE_TARGET.arch, x86_64){
    ARCHITECTURE = x64
} else {
    ARCHITECTURE = x86
}

linux {
    DEFINES += TARGET_OS_LINUX
    ARCHITECTURE = $${QMAKE_HOST.arch}
    COMPILER = gcc

	target.path = /usr/local/lib
	INSTALLS += target
}

DESTDIR = $${PWD}/bin/$${ARCHITECTURE}/$${COMPILER}



