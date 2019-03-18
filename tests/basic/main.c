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