cmake_minimum_required(VERSION 3.5)

project(BeatMagnifier)

FIND_PACKAGE(OpenCV 4.0 REQUIRED)

unset(CMAKE_CXX_FLAGS CACHE)

find_package(OpenMP)
if (OPENMP_FOUND)
    list(APPEND CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    list(APPEND CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
elseif(MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic-errors --std=c++14" CACHE STRING COMPILE_FLAGS FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native --fast-math -ffast-math -funroll-loops -Wall -DNDEBUG -DBOOST_DISABLE_ASSERTS" CACHE STRING COMPILE_FLAGS FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -march=native -mtune=native -Wall -DDEBUG" CACHE STRING COMPILE_FLAGS FORCE)
    add_definitions(-DUSE_OPENCV -DDLL_BUILD_SETUP)
elseif (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /W4" CACHE STRING COMPILE_FLAGS FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:AVX /fp:fast /O2 /Ob2 /Oi /Ot" CACHE STRING COMPILE_FLAGS FORCE)
    add_definitions(-DWIN32 -D_USE_MATH_DEFINES -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -DUSE_OPENCV -DUSE_STATIC_IE -DDLL_BUILD_SETUP)
endif()

include_directories(${OpenCV_INCLUDE_DIRS})
# ----------------------------------------------------------------------

add_subdirectory(Library)
add_subdirectory(Examples)

# ----------------------------------------------------------------------

set(DATA_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Examples/haarcascade_frontalface_alt2.xml
)

if (CMAKE_COMPILER_IS_GNUCXX)
    file(COPY ${DATA_FILES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
elseif(MSVC)
    file(COPY ${DATA_FILES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
    file(COPY ${DATA_FILES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
endif()
