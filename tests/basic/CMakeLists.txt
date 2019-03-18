add_executable(basic_tests main.c)
target_link_libraries(basic_tests ${LIB_CFAKES})
add_test(NAME basic_tests COMMAND basic_tests)