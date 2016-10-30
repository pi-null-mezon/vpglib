TEMPLATE = lib

CONFIG(release, debug|release) {
    TARGET = vpg
} else {
    TARGET = vpgd
}

SOURCES += vpg.cpp

HEADERS += vpg.h

include(opencv.pri)
include(openmp.pri)
include(opencl.pri)

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

DESTDIR = $${PWD}/build/$${ARCHITECTURE}/$${COMPILER}


