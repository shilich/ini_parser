cmake_minimum_required(VERSION 3.16)
set(TARGET_NAME ini_parser)

project(${TARGET_NAME})
set(CMAKE_CXX_STANDARD 14)


set(HEADERS
    value.h
    parser.h
    errors.h
    )

set(SOURCES
    )

add_library(${TARGET_NAME} INTERFACE)
