cmake_minimum_required(VERSION 3.16)
project(CUnitTests VERSION 1.0.0 LANGUAGES C)

message("Adding ${PROJECT_NAME} library")
add_library(CUnitTests INTERFACE)
target_include_directories(CUnitTests INTERFACE include/)

if(BUILD_TESTING)
    message("${PROJECT_NAME} library testing enabled")
    add_executable(CUnitTestsExample tests/example.c)
    target_link_libraries(CUnitTestsExample CUnitTests)
    enable_testing()
    add_test(NAME CUnitTestsExample COMMAND CUnitTestsExample -e)
endif()
