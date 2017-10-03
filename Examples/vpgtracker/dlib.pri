# This pri files describes where Dlib's library located and how the app should be linked with the library
win32 {
    DLIB_INSTALL_PATH = C:/Programming/3rdParties/Dlib/build

    INCLUDEPATH += $${DLIB_INSTALL_PATH}/include

    LIBS += -L$${DLIB_INSTALL_PATH}/lib

    LIBS += -ldlib
}
