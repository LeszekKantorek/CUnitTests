# CUnitTests
Simple and robust, single header file ANSI C unit testing library.

## Simple test example

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
``` c
__CUnitTests_Error_Succeed = 0,
__CUnitTests_Error_Error = 1,
__CUnitTests_Error_Failed = 2,
__CUnitTests_Error_NotFound = 3,
__CUnitTests_Error_NotExecuted = 4,
```

## Assertions
Build-in assertions: 
``` c
test_assert_true(expr, message, ...)				
test_assert_false(expr, message, ...)				
test_assert_equal(expected, result, message, ...)	
test_assert_not_equal(expected, result, message, ...)
test_assert_null(value, message, ...)
test_assert_not_null(value, message,...)
```
Use following functions to set given test result.
``` c
test_succeed()	- set test result to Succeed.
test_failed()	- set test result to Failed. 
```

## CMake installation
The easiest way to get library installed is to use the CMake FetchContent_Declare function.
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
