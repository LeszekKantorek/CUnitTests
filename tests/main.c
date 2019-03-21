#include "cfakes/cfakes.h"
#include "simple_test.c"
#include "assertion_tests.c"
int main(int argc, char **argv) {
	cfakes_initialize(argc, argv);

	cfakes_test_t tests_to_execute[] = {
		cfakes_test(simple_test),
		cfakes_test_setup(simple_test_2_routine, simple_test_2_setup),
		cfakes_test_setup_cleanup(simple_test_3_routine, simple_test_3_setup, simple_test_3_cleanup),

		cfakes_test(assert_true_succeed_test),
		cfakes_test(assert_true_failed_test),

		cfakes_test(assert_false_succeed_test),
		cfakes_test(assert_false_failed_test),

		cfakes_test(assert_equal_succeed_test),
		cfakes_test(assert_equal_failed_test),

		cfakes_test(assert_not_equal_succeed_test),
		cfakes_test(assert_not_equal_failed_test),

		cfakes_test(assert_null_succeed_test),
		cfakes_test(assert_null_failed_test),

		cfakes_test(assert_not_null_succeed_test),
		cfakes_test(assert_not_null_failed_test),
	};

	cfakes_result_t result = cfakes_run(tests_to_execute, argc, argv);
	return result;
}