project(A3DllProxy)
cmake_minimum_required(VERSION 2.8.6)

aux_source_directory(. SRC_LIST)

file (GLOB HEADER_LIST "./*.h")

message("My sources: ${SRC_LIST}")
message("My headers: ${HEADER_LIST}")

include_directories(.)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

IF (MSVC)

ELSE(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") # Warnings
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g") # features
ENDIF(MSVC)

IF(WIN32)
    add_definitions(-DWIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
ENDIF(WIN32)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST} ${HEADER_LIST})

