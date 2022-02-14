cmake_minimum_required(VERSION 3.16)
project(ini)

set(INI_PARSER_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()