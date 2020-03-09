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
program_name -e                  - execute all tests \
program_name -ie                 - execute all tests in isolation \
program_name -e first second ... - execute specified tests \
program_name /i first second ... - execute specified tests in isolation\
program_name                     - list all tests
```

## Tests executable exit codes
The test program uses following exit codes:
* 0 - Success
* 1 - Test not found, Test not executed, error 
* 2 - Failure

## Assertions
Build-in assertions: 
``` c
test_assert_true(expr, message, ...);				
test_assert_false(expr, message, ...);				
test_assert_equal(expected, result, message, ...);	
test_assert_not_equal(expected, result, message, ...);
test_assert_null(value, message, ...);
test_assert_not_null(value, message,...);
```
Use following functions to set given test result.
``` c
test_succeed()	- set test result to Succeed.
test_failed()	- set test result to Failed. 
```

## CMake installation
The easiest way to get library installed is to use the CMake `FetchContent_Declare` function.
``` CMake
include(FetchContent)
FetchContent_Declare(CUnitTests
  GIT_REPOSITORY https://github.com/LeszekKantorek/CUnitTests.git
  GIT_TAG v1.0  # use tags for version or master for the latest version 
)
FetchContent_MakeAvailable(CUnitTests)

add_executable(YourTestApp tests/main.c)
target_link_libraries(YourTestApp CUnitTests)
```
