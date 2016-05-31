#CONFIG += openmp
openmp {
    message(OpenMP enabled)
    msvc* {
        QMAKE_CXXFLAGS+= -openmp
    }
    g++ {
        QMAKE_CXXFLAGS+= -fopenmp
        LIBS += -fopenmp
    }
}
