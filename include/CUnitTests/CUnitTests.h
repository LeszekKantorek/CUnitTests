/*
MIT License

Copyright (c) 2020 Leszek Kantorek

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _CUnitTests_CUnitTests_h_
#define _CUnitTests_CUnitTests_h_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define __CUNIT_TESTS_MAX 1024
#define __CUNIT_TESTS_RUN_COMMAND_PATTERN "%s -eq %s"
#define __CUNIT_TESTS_MAX_TEST_SUITES_FILTER 10

typedef enum __CUnitTestsResult {
	__CUnitTestsResult_SUCCEED = 0,
	__CUnitTestsResult_ERROR = 1,
	__CUnitTestsResult_FAILED = 2,
	__CUnitTestsResult_NOT_EXECUTED = 3,
} __CUnitTestsResult;

typedef enum __CUnitTestsAction {
	__CUnitTestsAction_USAGE = 0,
	__CUnitTestsAction_LIST_TESTS = 1,
	__CUnitTestsAction_EXECUTE = 2
} __CUnitTestsAction;

typedef enum __CUnitTestsExecutionMode {
	__CUnitTestsExecutionMode_IN_PROCESS = 0,
	__CUnitTestsExecutionMode_NEW_PROCESS = 1
} __CUnitTestsExecutionMode;

typedef struct __CUnitTestsTest {
	char *testName;
	char *suite;
	void (*setup)();
	void (*routine)();
	void (*cleanup)();
	__CUnitTestsResult result;
} __CUnitTestsTest;

typedef struct __CUnitTestsContext {
	char *executableName;
	__CUnitTestsAction action;
	__CUnitTestsExecutionMode mode;
	__CUnitTestsResult result;
	__CUnitTestsTest **tests;
	unsigned testsCount;
} __CUnitTestsContext;

typedef struct __CUnitTestsFormatting {
	unsigned COLORS;
	unsigned QUIET;
} __CUnitTestsFormatting;

static __CUnitTestsTest *__CUNIT_TESTS_ALL_TESTS[__CUNIT_TESTS_MAX];
static unsigned __CUNIT_TESTS_ALL_TESTS_COUNT = 0;
static __CUnitTestsTest *__CUNIT_TESTS_CURRENT_TEST = NULL;
static __CUnitTestsFormatting __CUNIT_TESTS_FORMATTING = {.COLORS = 0, .QUIET = 0};

static void __CUnitTests_PrintQuiet(char *format, ...) {
	if (__CUNIT_TESTS_FORMATTING.QUIET == 0) {
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
}

static char *__CUnitTests_GetExecutableName(char *filePath) {
	for (size_t i = strlen(filePath) - 1; i; i--) {
		if (filePath[i] == '/' || filePath[i] == '\\') {
			return &filePath[i + 1];
		}
	}
	return filePath;
}

#define __CUnitTests_GetColorString(color, value)                                                                      \
	__CUNIT_TESTS_FORMATTING.COLORS ? "\033[0;" #color "m" #value "\033[0m" : #value

static char *__CUnitTests_GetResultString(__CUnitTestsResult result) {
	switch (result) {
		case __CUnitTestsResult_SUCCEED: return __CUnitTests_GetColorString(32, SUCCEED);
		case __CUnitTestsResult_FAILED: return __CUnitTests_GetColorString(31, FAILED);
		case __CUnitTestsResult_NOT_EXECUTED: return __CUnitTests_GetColorString(31, NOT_EXECUTED);
		default: return __CUnitTests_GetColorString(31, ERROR);
	}
}

static unsigned __CUnitTests_FilterTestsBySuiteName(__CUnitTestsContext *ctx, char **suites, unsigned suitesCount,
													__CUnitTestsTest **input, unsigned inputCount,
													__CUnitTestsTest ***output) {
	if (suitesCount == 0) {
		*output = input;
		return inputCount;
	}

	__CUnitTestsTest *temp[__CUNIT_TESTS_MAX];
	unsigned testsCount = 0;

	for (unsigned s = 0; s < suitesCount; s++) {
		char *suite = suites[s];
		unsigned found = 0;
		for (unsigned i = 0; i < inputCount; i++) {
			__CUnitTestsTest *test = input[i];
			if (test->suite != NULL && strcasecmp(test->suite, suite) == 0) {
				temp[testsCount] = test;
				testsCount++;
				found++;
				continue;
			}
		}

		if (!found) {
			printf("Test suite '%s' not found.\n", suite);
			ctx->result = __CUnitTestsResult_ERROR;
		}
	}

	*output = temp;
	return testsCount;
}

static unsigned __CUnitTests_FilterTestsByTestName(__CUnitTestsContext *ctx, char **names, unsigned namesCount,
												   __CUnitTestsTest **input, unsigned inputCount,
												   __CUnitTestsTest ***output) {
	if (namesCount == 0) {
		*output = input;
		return inputCount;
	}

	__CUnitTestsTest *temp[__CUNIT_TESTS_MAX];
	unsigned testsCount = 0;

	for (unsigned n = 0; n < namesCount; n++) {
		char *name = names[n];
		unsigned found = 0;
		for (unsigned i = 0; i < inputCount; i++) {
			__CUnitTestsTest *test = input[i];
			if (strcasecmp(test->testName, name) == 0) {
				temp[testsCount] = test;
				testsCount++;
				found++;
				break;
			}
		}

		if (!found) {
			printf("Test '%s' not found.\n", name);
			ctx->result = __CUnitTestsResult_ERROR;
		}
	}

	*output = temp;
	return testsCount;
}

static void __CUnitTests_FilterTests(__CUnitTestsContext *ctx, char **suites, unsigned suitesCount, char **names,
									 unsigned namesCount, __CUnitTestsTest **tests, unsigned testsCount) {
	if (suitesCount == 0 && namesCount == 0) {
		ctx->tests = tests;
		ctx->testsCount = testsCount;
		return;
	}

	__CUnitTestsTest **bySuite = NULL;
	unsigned bySuiteCount = __CUnitTests_FilterTestsBySuiteName(ctx, suites, suitesCount, tests, testsCount, &bySuite);

	__CUnitTestsTest **byName = NULL;
	unsigned byNameCount = __CUnitTests_FilterTestsByTestName(ctx, names, namesCount, bySuite, bySuiteCount, &byName);

	ctx->tests = byName;
	ctx->testsCount = byNameCount;
}

#define __CUnitTests_SetContextAction(ctx, newAction)                                                                  \
	if (ctx->action == __CUnitTestsAction_USAGE) {                                                                     \
		ctx->action = newAction;                                                                                       \
	}

static __CUnitTestsContext *__CUnitTests_CreateContext(int argc, char *argv[]) {
	__CUnitTestsContext *ctx = malloc(sizeof(__CUnitTestsContext));
	ctx->executableName = argv[0];
	ctx->action = __CUnitTestsAction_USAGE;
	ctx->mode = __CUnitTestsExecutionMode_IN_PROCESS;
	ctx->result = __CUnitTestsResult_SUCCEED;

	char *suites[__CUNIT_TESTS_MAX_TEST_SUITES_FILTER];
	unsigned suitesCount = 0;

	__CUNIT_TESTS_FORMATTING.COLORS = false;
	__CUNIT_TESTS_FORMATTING.QUIET = false;

	int opt;
	while ((opt = getopt(argc, argv, "eiclqs:")) != -1) {
		switch (opt) {
			case 'l': __CUnitTests_SetContextAction(ctx, __CUnitTestsAction_LIST_TESTS); break;
			case 'c': __CUNIT_TESTS_FORMATTING.COLORS = true; break;
			case 'e': __CUnitTests_SetContextAction(ctx, __CUnitTestsAction_EXECUTE); break;
			case 'i': ctx->mode = __CUnitTestsExecutionMode_NEW_PROCESS; break;
			case 'q': __CUNIT_TESTS_FORMATTING.QUIET = true; break;
			case 's':
				if (suitesCount < __CUNIT_TESTS_MAX_TEST_SUITES_FILTER) {
					char *suiteName = malloc(strlen(optarg));
					strcpy(suiteName, optarg);
					suites[suitesCount++] = suiteName;
				}
				break;
			default: break;
		}
	}

	int namesCount = argc - optind;
	char **names = &argv[optind];
	__CUnitTests_FilterTests(ctx, suites, suitesCount, names, namesCount, __CUNIT_TESTS_ALL_TESTS,
							 __CUNIT_TESTS_ALL_TESTS_COUNT);
	return ctx;
}

static void __CUnitTests_ExecuteTestsInProcess(__CUnitTestsContext *ctx) {
	__CUnitTests_PrintQuiet("Executing '%s' tests in process", ctx->executableName);

	for (unsigned int testIndex = 0; testIndex < ctx->testsCount; testIndex++) {
		__CUnitTestsTest *test = ctx->tests[testIndex];
		if (test->result == __CUnitTestsResult_NOT_EXECUTED) {
			test->result = __CUnitTestsResult_SUCCEED;
			__CUNIT_TESTS_CURRENT_TEST = test;
			__CUnitTests_PrintQuiet("\n'%s' started...", test->testName);
			if (test->setup != NULL) {
				test->setup();
			}
			test->routine();
			if (test->cleanup != NULL) {
				test->cleanup();
			}

			__CUnitTests_PrintQuiet("\n'%s' %s", test->testName, __CUnitTests_GetResultString(test->result));
		}
	}
}

static __CUnitTestsResult __CUnitTests_GetTestProcessResult(int exitCode) {
	switch (exitCode) {
		case 0: return __CUnitTestsResult_SUCCEED;
		case 2: return __CUnitTestsResult_FAILED;
		case 3: return __CUnitTestsResult_NOT_EXECUTED;
		default: return __CUnitTestsResult_ERROR;
	}
}

static char *__CUnitTests_PrepareTestCommand(__CUnitTestsContext *ctx, __CUnitTestsTest *test) {
	int cmdSize = snprintf(NULL, 0, __CUNIT_TESTS_RUN_COMMAND_PATTERN, ctx->executableName, test->testName) + 1;
	char *cmd = malloc(cmdSize);
	snprintf(cmd, cmdSize, __CUNIT_TESTS_RUN_COMMAND_PATTERN, ctx->executableName, test->testName);
	return cmd;
}

static __CUnitTestsResult __CUnitTests_RunTestCommand(char *cmd) {
	__CUnitTestsResult result = __CUnitTestsResult_NOT_EXECUTED;
	FILE *testProcess = popen(cmd, "r");
	if (testProcess != NULL) {
		char output[1000];
		while (fgets(output, sizeof(output), testProcess) != NULL) {
			printf("%s", output);
		}
		result = __CUnitTests_GetTestProcessResult(pclose(testProcess));
	}
	return result;
}

static void __CUnitTests_ExecuteTestsInNewProcess(__CUnitTestsContext *ctx) {
	__CUnitTests_PrintQuiet("Executing '%s' tests as separate processes", ctx->executableName);

	for (unsigned t = 0; t < ctx->testsCount; t++) {
		__CUnitTestsTest *test = ctx->tests[t];
		if (test->result == __CUnitTestsResult_NOT_EXECUTED) {
			test->result = __CUnitTestsResult_SUCCEED;
			char *cmd = __CUnitTests_PrepareTestCommand(ctx, test);
			__CUnitTests_PrintQuiet("\n'%s' started...", test->testName);
			__CUnitTests_RunTestCommand(cmd);
			__CUnitTests_PrintQuiet("\n'%s' %s", test->testName, __CUnitTests_GetResultString(test->result));
			free(cmd);
		}
	}
}

static void __CUnitTests_CheckExecutionResults(__CUnitTestsContext *ctx) {
	unsigned tests_succeed = 0;
	unsigned tests_failed = 0;
	unsigned tests_errored = 0;

	for (unsigned testIndex = 0; testIndex < ctx->testsCount; testIndex++) {
		__CUnitTestsTest *test = ctx->tests[testIndex];
		switch (test->result) {
			case __CUnitTestsResult_SUCCEED: tests_succeed++; break;
			case __CUnitTestsResult_FAILED: tests_failed++; break;
			default: tests_errored++; break;
		}
	}

	if (tests_errored > 0) {
		ctx->result = __CUnitTestsResult_ERROR;
	} else if (tests_failed > 0) {
		ctx->result = __CUnitTestsResult_FAILED;
	} else if (tests_succeed > 0) {
		ctx->result = __CUnitTestsResult_SUCCEED;
	}

	if (!__CUNIT_TESTS_FORMATTING.QUIET) {
		if (tests_failed || tests_errored) {
			printf("\n%s tests execution failures:", ctx->executableName);
			for (unsigned testIndex = 0; testIndex < ctx->testsCount; testIndex++) {
				__CUnitTestsTest *test = ctx->tests[testIndex];
				if (test->result != __CUnitTestsResult_SUCCEED) {
					printf("\n'%s':\t%s", test->testName, __CUnitTests_GetResultString(test->result));
				}
			}
		}

		printf("\n'%s' result: %s. Total: %u. Succeed: %u. Failed: %u. Errors: %u\n", ctx->executableName,
			   __CUnitTests_GetResultString(ctx->result), ctx->testsCount, tests_succeed, tests_failed, tests_errored);
	}
}

static void __CUnitTests_ExecuteTests(__CUnitTestsContext *ctx) {
	if (ctx->result == __CUnitTestsResult_SUCCEED) {
		if (ctx->mode == __CUnitTestsExecutionMode_IN_PROCESS) {
			__CUnitTests_ExecuteTestsInProcess(ctx);
		} else {
			__CUnitTests_ExecuteTestsInNewProcess(ctx);
		}
	}
	__CUnitTests_CheckExecutionResults(ctx);
}

static void __CUnitTests_PrintUsage(__CUnitTestsContext *ctx) {
	printf("Usage:\n");
	printf("%s -e                       - execute all tests\n", ctx->executableName);
	printf("%s -ei                      - execute all tests as separate processes\n", ctx->executableName);
	printf("%s -e first second          - execute selected tests\n", ctx->executableName);
	printf("%s -ei first second         - execute selected tests as separate processes\n", ctx->executableName);
	printf("%s -e -s first -s second    - execute selected test suites\n", ctx->executableName);
	printf("%s -l                       - list all tests\n", ctx->executableName);
	printf("%s                          - print usage\n", ctx->executableName);
	printf("\nAdditional flags:\n");
	printf("-c                          - color output\n");
	printf("-q                          - quiet mode (no tests summaries)\n");
}

static void __CUnitTests_ListTests() {
	for (unsigned index = 0; index < __CUNIT_TESTS_ALL_TESTS_COUNT; index++) {
		printf("%s\n", __CUNIT_TESTS_ALL_TESTS[index]->testName);
	}
}

static __CUnitTestsResult __CUnitTests_Perform(__CUnitTestsContext *ctx) {
	switch (ctx->action) {
		case __CUnitTestsAction_LIST_TESTS: __CUnitTests_ListTests(); break;
		case __CUnitTestsAction_EXECUTE: __CUnitTests_ExecuteTests(ctx); break;
		default: __CUnitTests_PrintUsage(ctx); break;
	}
	return ctx->result;
}

int main(int argc, char *argv[]) {
	__CUnitTestsContext *ctx = __CUnitTests_CreateContext(argc, argv);
	__CUnitTestsResult result = __CUnitTests_Perform(ctx);
	return result;
}

static void __CUnitTests_SetTestResult(char *file, int line, __CUnitTestsResult result) {
	printf("\n\tChanging test result to %s in %s:%d", __CUnitTests_GetResultString(result), file, line);
	__CUNIT_TESTS_CURRENT_TEST->result = __CUnitTestsResult_SUCCEED;
}

static void __CUnitTests_AssertionFailed(char *file, int line, char *message, ...) {
	__CUNIT_TESTS_CURRENT_TEST->result = __CUnitTestsResult_FAILED;
	va_list args;
	va_start(args, message);
	printf("\n\tAssertion failed: '");
	vprintf(message, args);
	printf("' in %s:%d", file, line);
	va_end(args);
}

#define test(name, ...)                                                                                                \
	void __CUnitTests_Generated_Routine_##name();                                                                      \
	static __CUnitTestsTest __CUnitTests_Generated_Test_##name = {.testName = #name,                                   \
																  .routine = &__CUnitTests_Generated_Routine_##name,   \
																  .result = __CUnitTestsResult_NOT_EXECUTED,           \
																  __VA_ARGS__};                                        \
	__attribute__((constructor)) void ___CUnitTests_Generated_Register_##name() {                                      \
		__CUNIT_TESTS_ALL_TESTS[__COUNTER__] = &__CUnitTests_Generated_Test_##name;                                    \
		__CUNIT_TESTS_ALL_TESTS_COUNT++;                                                                               \
	}                                                                                                                  \
	void __CUnitTests_Generated_Routine_##name()

#define test_set_failed() __CUnitTests_SetTestResult(__FILE__, __LINE__, __CUnitTestsResult_FAILED)
#define test_set_succeed() __CUnitTests_SetTestResult(__FILE__, __LINE__, __CUnitTestsResult_SUCCEED)
#define test_failed() __CUNIT_TESTS_CURRENT_TEST->result == __CUnitTestsResult_FAILED

#define test_assert_true(expr)                                                                                         \
	if (!(expr)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, #expr)

#define test_assert_true_fmt(expr, message, ...)                                                                       \
	if (!(expr)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_false(expr)                                                                                        \
	if ((expr)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, #expr)

#define test_assert_false_fmt(expr, message, ...)                                                                      \
	if ((expr)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_equal(expected, result)                                                                            \
	if ((expected) != (result)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, "%s==%s", #expected, #result);

#define test_assert_equal_fmt(expected, result, message, ...)                                                          \
	if ((expected) != (result)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__);

#define test_assert_not_equal(expected, result)                                                                        \
	if ((expected) == (result)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, "%s!=%s", #expected, #result)

#define test_assert_not_equal_fmt(expected, result, message, ...)                                                      \
	if ((expected) == (result)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_null(value)                                                                                        \
	if ((value) != (NULL)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, "%s == NULL", #value)

#define test_assert_null_fmt(value, message, ...)                                                                      \
	if ((value) != (NULL)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_not_null(value)                                                                                    \
	if ((value) == (NULL)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, "%s != NULL", #value)

#define test_assert_not_null_fmt(value, message, ...)                                                                  \
	if ((value) == (NULL)) __CUnitTests_AssertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#endif /* _CUnitTests_CUnitTests_h_ */
