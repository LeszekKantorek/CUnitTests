cmake_minimum_required (VERSION 3.14)

set(CFAKES_LIB_NAME cfakes)
include(build/settings.cmake)

project(${CFAKES_LIB_NAME} C)
add_subdirectory(src)
add_subdirectory(tests)