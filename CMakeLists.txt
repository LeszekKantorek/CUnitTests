cmake_minimum_required (VERSION 3.14)
set(CFAKES_LIB cfakes)
project(${CFAKES_LIB} LANGUAGES C)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)