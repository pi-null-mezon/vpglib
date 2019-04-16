cmake_minimum_required(VERSION 3.5)

project(vpglight)

add_definitions(-DAPP_DESIGNER=\"Alex.A.Taranov\" -DAPP_NAME=\"${PROJECT_NAME}\" -DAPP_VERSION=\"1.0.0.5\")

include_directories(${OpenCV_INCLUDE_DIRS}
                    ${CMAKE_SOURCE_DIR}/Library/include)

# ----------------------------------------------------------------------
set(SOURCE
    main.cpp
)

set(HEADERS
)

set(LIBS
    ${OpenCV_LIBS}
    vpg
)

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${LIBS})

if (CMAKE_COMPILER_IS_GNUCXX)
    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
elseif(MSVC)
    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()
