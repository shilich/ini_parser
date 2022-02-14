cmake_minimum_required(VERSION 3.16)
set(TARGET_NAME test_ini_parser)

project(${TARGET_NAME})
set(CMAKE_CXX_STANDARD 14)

find_package (Boost REQUIRED COMPONENTS unit_test_framework)

add_executable(${TARGET_NAME} valuetest.cpp teststructures.h parsertest.cpp)

target_link_libraries(${TARGET_NAME} PRIVATE ini_parser Boost::unit_test_framework)
target_include_directories(${TARGET_NAME} PRIVATE ${INI_PARSER_ROOT}/src ${Boost_INCLUDE_DIRS})

