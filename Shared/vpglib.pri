VPGLIBPATH = $${PWD}/../Library

defineReplace(qtLibraryName) {
   unset(LIBRARY_NAME)
   LIBRARY_NAME = $$1
   CONFIG(debug, debug|release): RET = $$member(LIBRARY_NAME, 0)d
   isEmpty(RET):RET = $$LIBRARY_NAME
   return($$RET)
}

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
    DEFINES += Q_OS_LINUX
    ARCHITECTURE  = arm32
    COMPILER = gcc
}

LIBS += -L$${VPGLIBPATH}/bin/$${ARCHITECTURE}/$${COMPILER}
LIBS += -l$$qtLibraryName(vpg)

INCLUDEPATH += $${VPGLIBPATH}/include

include($${PWD}/opencv.pri)
