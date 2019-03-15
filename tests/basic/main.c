#include <stdlib.h>
#include <assert.h>
#include "cfakes/cfakes.h"
#include "simple_test.c"

int main(int argc, char** argv) {
	
	cfakes_settings.execute_tests_in_isolation = 1;

	cfakes_unit_test_t tests_suite[] =
	{
		cfakes_unit_test_setup_cleanup(simple_test, simple_test_setup, simple_test_cleanup),
	};
	
	cfakes_result_t result = cfakes_execute_tests(tests_suite, argc, argv);
	assert(result == CFAKES_TEST_RESULT_SUCCEED);
	return result;
}