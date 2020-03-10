# CUnitTests
Simple and robust, single header file ANSI C unit testing library.

## Defining tests
Use the `test(TestName, {test code});` macro to add test.

example.c
``` c
#include "CUnitTests/CUnitTests.h"
test(First, { printf("From test 1"); });
```

## Tests executable usage
```
program_name -e                   - execute all tests \
program_name -ei                  - execute all tests in isolation \
program_name -e first second ...  - execute specified tests \
program_name -ei first second ... - execute specified tests in isolation\
program_name                      - list all tests
```
The term 'in isolation' means as separate process.

## Tests executable exit codes
The test program exit codes:
```
0 - Success
1 - Test not found, Test not executed, error 
2 - Failure
```

## API
Build-in test assertions: 
``` c
test_assert_true(expr, message, ...);				
test_assert_false(expr, message, ...);				
test_assert_equal(expected, result, message, ...);	
test_assert_not_equal(expected, result, message, ...);
test_assert_null(value, message, ...);
test_assert_not_null(value, message,...);
```
Seting given test result from code:
``` c
test_set_succeed();    // changes current test result to Succeed.
test_set_failed();     // changes current test result to Failed. 
```
Additional macros:
``` c
test_failed();                    // checks if given test has failed.
test_print_info(format, args);    // prints formatted message into stdout.
test_print_error(format, args);   // prints formatted error message into stderr.
```

example.c
``` c
test(check_failure,{
    ...
    test_assert_true(0, "Should be true");
    if(test_failed()){
        return;
    }
    ...
});
```

## CMake installation
The easiest way to get library installed is to use the CMake `FetchContent_Declare` function.
``` CMake
include(FetchContent)
FetchContent_Declare(CUnitTests
  GIT_REPOSITORY https://github.com/LeszekKantorek/CUnitTests.git
  GIT_TAG v1.1  # use tags for version or master for the latest version 
)
FetchContent_MakeAvailable(CUnitTests)

add_executable(YourTestApp tests/main.c)
target_link_libraries(YourTestApp CUnitTests)
```
