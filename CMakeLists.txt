cmake_minimum_required (VERSION 3.14)
set(LIB_CFAKES cfakes)
project(${LIB_CFAKES} LANGUAGES C)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)