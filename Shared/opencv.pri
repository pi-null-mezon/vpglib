#--------------------------------------------------------OPENCV----------------------------------------------------
#Specify a path to the build directory of opencv library and library version
win32 {
    OPENCV_VERSION = 310 # 330 works also
    OPENCV_DIR = C:/Programming/3rdParties/opencv$${OPENCV_VERSION}/build
    INCLUDEPATH += $${OPENCV_DIR}/include

    #Specify the part of OpenCV path corresponding to compiler version
    win32-msvc2010: OPENCV_COMPILER = vc10
    win32-msvc2012: OPENCV_COMPILER = vc11
    win32-msvc2013: OPENCV_COMPILER = vc12
    win32-msvc2015: OPENCV_COMPILER = vc14
    win32-g++:      OPENCV_COMPILER = mingw

    #Specify the part of OpenCV path corresponding to target architecture
    win32:contains(QMAKE_TARGET.arch, x86_64){
        OPENCV_ARCHITECTURE = x64
    } else {
        OPENCV_ARCHITECTURE = x86
    }

    #A tricky way to resolve debug and release library versions
    defineReplace(qtLibraryName) {
       unset(LIBRARY_NAME)
       LIBRARY_NAME = $$1
       CONFIG(debug, debug|release): RET = $$member(LIBRARY_NAME, 0)d
       isEmpty(RET):RET = $$LIBRARY_NAME
       return($$RET)
    }

    #Specify path to *.lib files
    win32-msvc*:LIBS += -L$${OPENCV_DIR}/$${OPENCV_ARCHITECTURE}/$${OPENCV_COMPILER}/lib/
    win32-msvc*:LIBS += -L$${OPENCV_DIR}/$${OPENCV_ARCHITECTURE}/$${OPENCV_COMPILER}/bin/
    win32-g++:  LIBS += -L$${OPENCV_DIR}/$${OPENCV_ARCHITECTURE}/$${OPENCV_COMPILER}/bin/

    #Specify names of *.lib files
    LIBS += -l$$qtLibraryName(opencv_core$${OPENCV_VERSION}) \
            -l$$qtLibraryName(opencv_highgui$${OPENCV_VERSION}) \
            -l$$qtLibraryName(opencv_imgproc$${OPENCV_VERSION}) \
            -l$$qtLibraryName(opencv_objdetect$${OPENCV_VERSION}) \
            -l$$qtLibraryName(opencv_videoio$${OPENCV_VERSION})

    DEFINES += OPENCV_DATA_DIR=\\\"$${OPENCV_DIR}/../sources/data\\\"

    message(OpenCV version: $${OPENCV_DIR}/$${OPENCV_ARCHITECTURE}/$${OPENCV_COMPILER})
}

linux {
    DEFINES += OPENCV_DATA_DIR=\\\"/home/pi/Programming/3rdParties/opencv310/sources/data\\\"
    LIBS += -L/usr/local/lib
    LIBS += -L/usr/local/bin
    LIBS += -lopencv_core \
            -lopencv_objdetect \
            -lopencv_highgui \
            -lopencv_imgproc \
            -lopencv_videoio
}
