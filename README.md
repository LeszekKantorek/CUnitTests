# cfakes
Single header file ANSI C unit testing library.

## Simple test example

main.c
``` main.c
#include <stdlib.h>
#include <assert.h>
#include "cfakes/cfakes.h"
#include "simple_test.c"

int main(int argc, char** argv) {
	cfakes_initialize(argc, argv);
	
	cfakes_unit_test_t tests_to_execute[] =	{
		cfakes_unit_test_setup_cleanup(simple_test, simple_test_setup, simple_test_cleanup),
	};
	
	cfakes_result_t result = cfakes_run(tests_to_execute, argc, argv);
	return result;
}
```

simple_test.c
``` c
void simple_test_setup(){
}

void simple_test(){
}

void simple_test_cleanup(){
}
```

## Tests executable usage
program_name                  - run all tests in single process\
program_name /i               - run all tests as separate processes\
program_name first second ... - run selected tests\
program_name /i first second  - run selected tests as separate processes\
program_name /l               - list all tests

## Tests executable return codes
CFAKES_TEST_RESULT_SUCCEED = 0\
CFAKES_TEST_RESULT_UNKNOWN = 1\
CFAKES_TEST_RESULT_FAILED = 2\
CFAKES_TEST_RESULT_NOT_FOUND = 3