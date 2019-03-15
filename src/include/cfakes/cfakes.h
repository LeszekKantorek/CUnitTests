#ifndef _cfakes_cfakes_h_
#define _cfakes_cfakes_h_

#include <stdio.h>

#define __CFAKES_STR(str) #str
#define _CFAKES_STR(str) __CFAKES_STR(str)

typedef enum cfakes_result_t {
	CFAKES_TEST_RESULT_SUCCEED = 0,
	CFAKES_TEST_RESULT_UNKNOWN = 1,
	CFAKES_TEST_RESULT_FAILED = 2,
	CFAKES_TEST_RESULT_NOT_FOUND = 3,
}cfakes_result_t;

typedef struct cfakes_unit_test_t {
	char* name;
	void(*test_routine)();
	void(*setup_routine)();
	void(*cleanup_routine)();
	cfakes_result_t result;
}cfakes_unit_test_t;

typedef struct cfakes_settings_t {
	int execute_tests_in_isolation;
}cfakes_settings_t;

typedef struct _cfakes_context_t {
	char* executable_name;
	size_t tests_to_run;
	cfakes_result_t current_test_result;
}_cfakes_context_t;

static _cfakes_context_t _cfakes_context = { .executable_name = NULL,.current_test_result = CFAKES_TEST_RESULT_UNKNOWN, .tests_to_run = 0};
static cfakes_settings_t cfakes_settings = { .execute_tests_in_isolation = 0 };

#define cfakes_unit_test(test_routine) {_CFAKES_STR(test_routine), &test_routine, NULL, NULL, CFAKES_TEST_RESULT_SUCCEED}
#define cfakes_unit_test_setup(test_routine, setup_routine) {_CFAKES_STR(test_routine), &test_routine, &setup_routine, NULL, CFAKES_TEST_RESULT_SUCCEED}
#define cfakes_unit_test_setup_cleanup(test_routine, setup_routine, cleanup_routine) {_CFAKES_STR(test_routine), &test_routine, &setup_routine, &cleanup_routine, CFAKES_TEST_RESULT_SUCCEED}

static cfakes_result_t _cfakes_execute_test_in_process(cfakes_unit_test_t *test) {
	_cfakes_context.current_test_result = CFAKES_TEST_RESULT_UNKNOWN;
	if (test->setup_routine != NULL) {
		test->setup_routine();
	}
	if (test->test_routine != NULL) {
		test->test_routine();
	}
	if (test->cleanup_routine != NULL) {
		test->cleanup_routine();
	}
	if (_cfakes_context.current_test_result == CFAKES_TEST_RESULT_UNKNOWN) {
		_cfakes_context.current_test_result = CFAKES_TEST_RESULT_SUCCEED;
	}
	return _cfakes_context.current_test_result;
}

#define  _CFAKES_RUN_TEST_COMMAND_FORMAT "\"%s\" %s"

static cfakes_result_t _cfakes_execute_test_in_isolation(cfakes_unit_test_t *test) {
	int test_command_size = snprintf(NULL, 0, _CFAKES_RUN_TEST_COMMAND_FORMAT, _cfakes_context.executable_name, test->name);
	++test_command_size;
	char* test_command = malloc(test_command_size);
	snprintf(test_command, test_command_size, _CFAKES_RUN_TEST_COMMAND_FORMAT, _cfakes_context.executable_name, test->name);
	int test_command_result = system(test_command);
	free(test_command);

	return test_command_result >= CFAKES_TEST_RESULT_SUCCEED && test_command_result <= CFAKES_TEST_RESULT_NOT_FOUND ? test_command_result : CFAKES_TEST_RESULT_UNKNOWN;
}

static cfakes_result_t _cfakes_execute_test(cfakes_unit_test_t *test) {
	cfakes_result_t result = CFAKES_TEST_RESULT_UNKNOWN;
	if (cfakes_settings.execute_tests_in_isolation) {
		result = _cfakes_execute_test_in_isolation(test);
	}
	else {
		result = _cfakes_execute_test_in_process(test);
	}

	switch (result) {
	case CFAKES_TEST_RESULT_SUCCEED:
		printf(" ...SUCCEED\n");
		break;
	case CFAKES_TEST_RESULT_FAILED:
		printf(" ...FAILED\n", test->name);
		break;
	case CFAKES_TEST_RESULT_NOT_FOUND:
		printf(" ...NOT_FOUND\n", test->name);
		break;
	default:
		printf(" ...RESULT_UNKNOWN\n", test->name);
	}
	return result;
}

static void _cfakes_execute_all(cfakes_unit_test_t *tests, size_t tests_count) {
	for (size_t test = 0; test < tests_count; ++test) {
		printf("Test '%s'", tests->name);
		tests->result = _cfakes_execute_test(tests);
		tests++;
	}
}

static void _cfakes_find_and_execute(cfakes_unit_test_t *tests, size_t tests_count, int argc, char** argv) {
}

static cfakes_result_t _cfakes_verify_results(cfakes_unit_test_t *tests, size_t tests_count) {
	unsigned tests_succeed = 0;
	unsigned tests_unknown = 0;
	unsigned tests_failed = 0;
	unsigned tests_not_found = 0;

	printf("\n------------ Summary ------------");

	for (size_t test = 0; test < tests_count; ++test) {
		switch (tests->result) {
		case CFAKES_TEST_RESULT_SUCCEED:
			tests_succeed++;
			break;
		case CFAKES_TEST_RESULT_FAILED:
			printf("FAILED: '%s'\n", tests->name);
			tests_failed++;
			break;
		case CFAKES_TEST_RESULT_NOT_FOUND:
			printf("NOT_FOUND: '%s'\n", tests->name);
			tests_not_found++;
			break;
		default:
			tests_unknown++;
			printf("RESULT_UNKNOWN: '%s'\n", tests->name);
		}
		tests++;
	}

	printf("\nTotal: %d. Succeed: %d. Failed: %d. Not found: %d. Unknown: %d ", (int)tests_count, tests_succeed, tests_failed, tests_not_found, tests_not_found);

	if (tests_unknown > 0) {
		return CFAKES_TEST_RESULT_UNKNOWN;
	}

	if (tests_not_found > 0) {
		return CFAKES_TEST_RESULT_NOT_FOUND;
	}

	if (tests_failed > 0) {
		return CFAKES_TEST_RESULT_FAILED;
	}

	return CFAKES_TEST_RESULT_SUCCEED;
}

static cfakes_result_t _cfakes_execute_tests(cfakes_unit_test_t *tests, size_t tests_count, int argc, char** argv) {
	_cfakes_context.executable_name = argv[0];
	cfakes_result_t result = CFAKES_TEST_RESULT_SUCCEED;
	if (argc == 1) {
		_cfakes_execute_all(tests, tests_count);
	}
	else if (argc > 1) {
		_cfakes_find_and_execute(tests, tests_count, argc, argv);
	}
	return _cfakes_verify_results(tests, tests_count);
}
#define cfakes_execute_tests(tests, argc, argv) _cfakes_execute_tests(tests, sizeof tests / sizeof tests[0], argc, argv)

#endif /* _cfakes_cfakes_h_ */