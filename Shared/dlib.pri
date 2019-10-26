# This pri files describes where Dlib's library located and how the app should be linked with the library
win32 {
    #Specify the part of OpenCV path corresponding to compiler version
    win32-msvc2010: DLIB_COMPILER = vc10
    win32-msvc2012: DLIB_COMPILER = vc11
    win32-msvc2013: DLIB_COMPILER = vc12
    win32-msvc2015: DLIB_COMPILER = vc14
    win32-g++:      DLIB_COMPILER = mingw

    #Specify the part of OpenCV path corresponding to target architecture
    win32:contains(QMAKE_TARGET.arch, x86_64){
        DLIB_ARCHITECTURE = x64
    } else {
        DLIB_ARCHITECTURE = x86
    }

    DLIB_INSTALL_PATH = C:/Programming/3rdParties/Dlib/build_$${DLIB_COMPILER}$${DLIB_ARCHITECTURE}

    INCLUDEPATH += $${DLIB_INSTALL_PATH}/include

    LIBS += -L$${DLIB_INSTALL_PATH}/lib
}

linux {
    #if Dlib had been built with OpenBLAS
    CONFIG += openblasbackend
    openblasbackend {
        message(OpenBLAS backend enabled)
        LIBS += -lopenblas
    }
}

LIBS += -ldlib

DEFINES += PATH_TO_DLIB_RES=\\\"$${PATH_TO_DLIB_RESOURCES}\\\"
