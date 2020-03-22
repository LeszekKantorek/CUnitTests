cmake_minimum_required(VERSION 3.16)
project(CUnitTests VERSION 1.1 LANGUAGES C)

if(CUNITTESTS_FLAGS)
    message("Adding ${PROJECT_NAME} flags")
    string(APPEND CMAKE_C_FLAGS " -Werror -Wall -Wextra ")
    string(APPEND CMAKE_CXX_FLAGS " -Werror -Wall -Wextra ")

    string(APPEND CMAKE_C_FLAGS " -Wno-gnu-zero-variadic-macro-arguments")
    string(APPEND CMAKE_CXX_FLAGS " -Wno-gnu-zero-variadic-macro-arguments")
endif()

message("Adding ${PROJECT_NAME} library")
add_library(CUnitTests INTERFACE)
target_include_directories(CUnitTests INTERFACE include/)

if(CUNITTESTS_TESTING)
    message("Adding ${PROJECT_NAME} library test executable")
    add_executable(CUnitTestsExample tests/example.c)
    target_link_libraries(CUnitTestsExample CUnitTests)
    enable_testing()
    add_test(NAME CUnitTestsExample COMMAND CUnitTestsExample -e)
endif()
