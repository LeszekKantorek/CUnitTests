#ifndef _cfakes_cfakes_h_
#define _cfakes_cfakes_h_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define __CFAKES_STR(str) #str
#define _CFAKES_STR(str) __CFAKES_STR(str)
#define _CFAKES_RUN_TEST_COMMAND_FORMAT "\"%s\" %s >nul"

typedef enum cfakes_result_t {
	CFAKES_TEST_RESULT_SUCCEED = 0,
	CFAKES_TEST_RESULT_UNKNOWN = 1,
	CFAKES_TEST_RESULT_FAILED = 2,
	CFAKES_TEST_RESULT_NOT_FOUND = 3,
}cfakes_result_t;

typedef struct _cfakes_context_t {
	char* executable_name;
	int execute_tests_in_isolation;
	int list_tests;
	int arguments_count;
	cfakes_result_t current_test_result;
}_cfakes_context_t;

typedef struct cfakes_unit_test_t {
	char* name;
	void(*test_routine)();
	void(*setup_routine)();
	void(*cleanup_routine)();
	cfakes_result_t result;
}cfakes_unit_test_t;

static _cfakes_context_t _cfakes_context = { .executable_name = NULL,
											.current_test_result = CFAKES_TEST_RESULT_UNKNOWN,
											.execute_tests_in_isolation = 0,
											.list_tests = 0,
											.arguments_count = 1 };

static void _cfakes_print_help() {
	printf("cfakes tests library usage:\n");
	printf("program_name                  - run all tests in single process\n");
	printf("program_name /i               - run all tests in isolation\n");
	printf("program_name first second ... - run selected tests\n");
	printf("program_name /i first second  - run selected tests in isolation\n");
	printf("program_name /l               - list all tests\n");
}

static int _cfakes_parse_arguments(int argc, char** argv) {
	_cfakes_context.executable_name = argv[0];
	int result = 0;

	size_t arg_length = 0;
	int flag = 0;
	for (int arg = 1; arg < argc && result == 0; arg++) {
		switch ((int)argv[arg][0]) {
		case '-':
		case '/':
			arg_length = strlen(argv[arg]);
			if (arg_length == 2) {
				_cfakes_context.arguments_count++;
				flag = argv[arg][1];
				switch ((int)flag)
				{
				case 'i':
				case 'I':
					_cfakes_context.execute_tests_in_isolation = 1;
					break;
				case 'l':
				case 'L':
					_cfakes_context.list_tests = 1;
					break;
				default:
					result = 1;
					break;
				}
			}
			break;
		default:
			break;
		}
	}
	return result;
}

#define cfakes_initialize(argc, argv)  \
	if(_cfakes_parse_arguments(argc, argv)){ \
		_cfakes_print_help(); \
		return CFAKES_TEST_RESULT_UNKNOWN; \
	} \

#define cfakes_unit_test(test_routine) {_CFAKES_STR(test_routine), &test_routine, NULL, NULL, CFAKES_TEST_RESULT_SUCCEED}
#define cfakes_unit_test_setup(test_routine, setup_routine) {_CFAKES_STR(test_routine), &test_routine, &setup_routine, NULL, CFAKES_TEST_RESULT_SUCCEED}
#define cfakes_unit_test_setup_cleanup(test_routine, setup_routine, cleanup_routine) {_CFAKES_STR(test_routine), &test_routine, &setup_routine, &cleanup_routine, CFAKES_TEST_RESULT_SUCCEED}

static void _cfakes_list_tests(cfakes_unit_test_t *tests, size_t tests_count) {
	for (size_t test = 0; test < tests_count; ++test) {
		printf("%s\n", tests->name);
		tests++;
	}
}

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
	if (test->result != CFAKES_TEST_RESULT_NOT_FOUND) {
		if (_cfakes_context.execute_tests_in_isolation) {
			result = _cfakes_execute_test_in_isolation(test);
		}
		else {
			result = _cfakes_execute_test_in_process(test);
		}
	}
	else {
		result = CFAKES_TEST_RESULT_NOT_FOUND;
	}
	return result;
}

static cfakes_result_t _cfakes_verify_results(cfakes_unit_test_t *tests, size_t tests_count) {
	unsigned tests_succeed = 0;
	unsigned tests_unknown = 0;
	unsigned tests_failed = 0;
	unsigned tests_not_found = 0;

	printf("\n------------ Summary ------------\n");

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

	printf("\nTotal: %d. Succeed: %d. Failed: %d. Not found: %d. Unknown: %d ", (int)tests_count, tests_succeed, tests_failed, tests_not_found, tests_unknown);

	if (tests_unknown > 0) {
		return CFAKES_TEST_RESULT_UNKNOWN;
	}
	else if (tests_not_found > 0) {
		return CFAKES_TEST_RESULT_NOT_FOUND;
	}
	else if (tests_failed > 0) {
		return CFAKES_TEST_RESULT_FAILED;
	}

	return CFAKES_TEST_RESULT_SUCCEED;
}

static cfakes_result_t _cfakes_execute_all(cfakes_unit_test_t *tests, size_t tests_count) {

	if (_cfakes_context.execute_tests_in_isolation) {
		printf("Running tests in isolation\n");
	}
	cfakes_unit_test_t *test_to_execute = tests;
	for (size_t test = 0; test < tests_count; ++test) {
		printf("Executing '%s'", test_to_execute->name);
		test_to_execute->result = _cfakes_execute_test(test_to_execute);
		switch (test_to_execute->result) {
		case CFAKES_TEST_RESULT_SUCCEED:
			printf(" ...SUCCEED\n");
			break;
		case CFAKES_TEST_RESULT_FAILED:
			printf(" ...FAILED\n");
			break;
		case CFAKES_TEST_RESULT_NOT_FOUND:
			printf(" ...NOT_FOUND\n");
			break;
		default:
			printf(" ...RESULT_UNKNOWN\n");
		}
		test_to_execute++;
	}
	return _cfakes_verify_results(tests, tests_count);
}

static cfakes_result_t _cfakes_find_and_execute(cfakes_unit_test_t *tests, size_t tests_count, int argc, char** argv) {
	size_t selected_tests_count = argc - _cfakes_context.arguments_count;
	
	cfakes_unit_test_t *selected_tests = malloc(sizeof(cfakes_unit_test_t) * selected_tests_count);
	cfakes_unit_test_t *selected_tests_tmp = selected_tests;
	size_t found_tests_count = 0;
	int test_found = 0;

	for (int arg = _cfakes_context.arguments_count; arg < argc; ++arg) {
		cfakes_unit_test_t *test = tests;
		test_found = 0;
		for (size_t counter = 0; counter < tests_count; ++counter) {
			char *name = test->name;
			if (strcmp(name, argv[arg]) == 0) {
				*selected_tests_tmp = *test;
				selected_tests_tmp++;
				found_tests_count++;
				test_found = 1;
				break;
			}
			test++;
		}

		if (!test_found) {
			selected_tests_tmp->name = argv[arg];
			selected_tests_tmp->result = CFAKES_TEST_RESULT_NOT_FOUND;
			selected_tests_tmp++;
			found_tests_count++;
		}
	}

	cfakes_result_t result = _cfakes_execute_all(selected_tests, found_tests_count);
	free(selected_tests);
	return result;
}

static cfakes_result_t _cfakes_execute_tests(cfakes_unit_test_t *tests, size_t tests_count, int argc, char** argv) {
	cfakes_result_t result = CFAKES_TEST_RESULT_UNKNOWN;
	if (argc == _cfakes_context.arguments_count) {
		result = _cfakes_execute_all(tests, tests_count);
	}
	else if (argc > _cfakes_context.arguments_count) {
		result = _cfakes_find_and_execute(tests, tests_count, argc, argv);
	}
	return result;
}

static cfakes_result_t _cfakes_run(cfakes_unit_test_t *tests, size_t tests_count, int argc, char** argv) {
	if (_cfakes_context.list_tests) {
		_cfakes_list_tests(tests, tests_count);
		return CFAKES_TEST_RESULT_SUCCEED;
	}
	return _cfakes_execute_tests(tests, tests_count, argc, argv);
}

#define cfakes_run(tests, argc, argv) _cfakes_run(tests, sizeof tests / sizeof tests[0], argc, argv)

static void _cfakes_set_test_succeed(){
	_cfakes_context.current_test_result = CFAKES_TEST_RESULT_SUCCEED;
}

static void _cfakes_set_test_failed(){
	_cfakes_context.current_test_result = CFAKES_TEST_RESULT_FAILED;
}

static void _cfakes_assertion_failed(char *file, int line, char *message, ...){
	_cfakes_set_test_failed();
	va_list args;
    va_start(args, message);
	printf("\nAssertion failed: ");
	vprintf(message, args);
	printf(" File: %s Line: %d", file, line);
	va_end(args);
}

#define cfakes_assert_true(expr, message, ...) \
	if(!(expr)) \
		_cfakes_assertion_failed(__FILE__, __LINE__, message, ##__VA_ARGS__) \

#define cfakes_assert_false(expr, message, ...) \
	if((expr)) \
		_cfakes_assertion_failed(__FILE__, __LINE__, message, ##__VA_ARGS__) \

#define cfakes_assert_equal(expected, result, message, ...) \
    if ((expected)!=(result)) \
		_cfakes_assertion_failed(__FILE__, __LINE__, message, ##__VA_ARGS__); \

#define cfakes_assert_not_equal(expected, result, message, ...) \
    if ((expected)==(result)) \
		_cfakes_assertion_failed(__FILE__, __LINE__, message, ##__VA_ARGS__) \

#define cfakes_assert_null(value, message, ...) \
    if ((value)!=(NULL)) \
		_cfakes_assertion_failed(__FILE__, __LINE__, message, ##__VA_ARGS__) \

#define cfakes_assert_not_null(value, message,...) \
    if ((value)==(NULL)) \
		_cfakes_assertion_failed(__FILE__, __LINE__, message, ##__VA_ARGS__) \

#endif /* _cfakes_cfakes_h_ */
