cmake_minimum_required(VERSION 3.5)

project(libvpg)

set(SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/faceprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hrvprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/peakdetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pulseprocessor.cpp
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/faceprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/hrvprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/peakdetector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/pulseprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpg.h
)

include_directories(${OpenCV_INCLUDE_DIRS}
                    ${CMAKE_SOURCE_DIR}/Library/include)


add_library(vpg SHARED ${SOURCE} ${HEADERS})

set(LIBS
    ${OpenCV_LIBS}
)

target_link_libraries(vpg ${LIBS})

set_target_properties(vpg PROPERTIES FOLDER "libs")
