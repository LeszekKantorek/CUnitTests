cmake_minimum_required(VERSION 3.16)
project(CUnitTests VERSION 1.0.0 LANGUAGES C)

add_library(CUnitTests INTERFACE)
target_include_directories(CUnitTests INTERFACE headers/)

add_executable(CUnitTestsExample tests/example.c)
target_link_libraries(CUnitTestsExample CUnitTests)
