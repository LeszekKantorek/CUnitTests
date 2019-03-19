enable_testing()

set(TEST_FILES 
    simple_test.c
    assertion_tests.c)
set_source_files_properties(${TEST_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group(Tests FILES ${TEST_FILES})

add_executable(cfakes_tests main.c ${TEST_FILES})
target_link_libraries(cfakes_tests ${LIB_CFAKES})
add_test(NAME cfakes_tests_in_process COMMAND cfakes_tests)
