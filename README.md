Master:

[![Build Status](https://travis-ci.com/LeszekKantorek/CUnitTests.svg?branch=master)](https://travis-ci.com/LeszekKantorek/CUnitTests)

Develop: 

[![Build Status](https://travis-ci.com/LeszekKantorek/CUnitTests.svg?branch=develop)](https://travis-ci.com/LeszekKantorek/CUnitTests)

# CUnitTests
Simple and robust, single header file ANSI C unit testing library.

## Usage
Include the `CUnitTests.h` library and add new test using the `test(TestName, {test code});` macro.

example.c
``` c
#include "CUnitTests/CUnitTests.h"
test(First, { 
  printf("From test 1"); }
);
```

## Tests executable usage
```
test_executable -e                       - execute all tests
test_executable -ei                      - execute all tests as separate processes
test_executable -e first second ...      - execute selected tests
test_executable -ei first second ...     - execute selected tests as separate processes
test_executable -l                       - list all tests
test_executable                          - print usage

Additional flags:
-c                          - color output
-q                          - quiet mode (no tests summaries)
```

## Tests executable exit codes
The test program exit codes:
```
0 - Success
1 - Test not found, Test not executed, Errors
2 - Failure
```

## API
Build-in test assertions: 
``` c
test_assert_true(expr);
test_assert_true_fmt(expr, message, ...);

test_assert_false(expr);								
test_assert_false_fmt(expr, message, ...);				

test_assert_equal(expected, result);	
test_assert_equal_fmt(expected, result, message, ...);	

test_assert_not_equal(expected, result);
test_assert_not_equal_fmt(expected, result, message, ...);

test_assert_null(value, message);
test_assert_null_fmt(value, message, ...);

test_assert_not_null(value);
test_assert_not_null_fmt(value, message,...);
```

Additional macros:
``` c
test_set_succeed();               // changes current test result to SUCCEED.
test_set_failed();                // changes current test result to FAILED. 
test_failed();                    // checks if given test has failed.
```

example.c
``` c
test(check_failure,{
    unsigned result = 0;
    ...
    test_assert_true(result);
    if(test_failed()){
        return;
    }
});
```

## CMake installation
The easiest way to get library installed is to use the CMake `FetchContent_Declare` function.
``` CMake
include(FetchContent)
FetchContent_Declare(CUnitTests
  GIT_REPOSITORY https://github.com/LeszekKantorek/CUnitTests.git
  GIT_TAG master  # master for the latest or tags for the given version
)
FetchContent_MakeAvailable(CUnitTests)

add_executable(YourTestApp tests/main.c)
target_link_libraries(YourTestApp CUnitTests)
```
