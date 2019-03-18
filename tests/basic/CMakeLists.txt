add_executable(basic_tests main.c)
target_link_libraries(basic_tests ${CFAKES_LIB})
#target_include_directories(basic_tests PRIVATE ${CFAKES_LIB_INCLUDE_DIR})
add_test(NAME basic_tests COMMAND basic_tests)