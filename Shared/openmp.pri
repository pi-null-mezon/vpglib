#CONFIG += openmp
openmp {
    message(OpenMP enabled)
    win32-msvc* {
        QMAKE_CXXFLAGS+= -openmp
    }
    win32-g++ {
        QMAKE_CXXFLAGS+= -fopenmp
        LIBS += -fopenmp
    }
}
