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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define __CUNIT_TESTS_MAX 1024
#define __CUNIT_TESTS_RUN_COMMAND_PATTERN "%s -eq %s"
#define __CUNIT_TESTS_MAX_TEST_SUITES 10

typedef enum __CUnitTests_Error {
	__CUnitTests_Error_Succeed = 0,
	__CUnitTests_Error_Error = 1,
	__CUnitTests_Error_Failed = 2,
	__CUnitTests_Error_NotExecuted = 3,
} __CUnitTests_Error;

typedef struct __CUnitTests_Test {
	char *testName;
	char *suite;
	void (*setup)();
	void (*routine)();
	void (*cleanup)();
	__CUnitTests_Error result;
} __CUnitTests_Test;

typedef enum __CUnitTests_Action {
	__CUnitTests_Action_PrintUsage = 0,
	__CUnitTests_Action_List = 1,
	__CUnitTests_Action_Execute = 2
} __CUnitTests_Action;

typedef enum __CUnitTests_ExecutionMode {
	__CUnitTests_ExecutionMode_InProcess = 0,
	__CUnitTests_ExecutionMode_NewProcess = 1
} __CUnitTests_ExecutionMode;

typedef struct __CUnitTests_Context {
	char *executableName;
	__CUnitTests_Action action;
	__CUnitTests_ExecutionMode executionMode;
	__CUnitTests_Error executionResult;
	__CUnitTests_Test **testsToExecute;
	unsigned testsToExecuteCount;
} __CUnitTests_Context;

static __CUnitTests_Test *__CUnitTests_Global_tests[__CUNIT_TESTS_MAX];
static unsigned __CUnitTests_Global_testsCount = 0;
static __CUnitTests_Test *__CUnitTests_Global_currentTest = NULL;
static unsigned __CUnitTests_Global_outputColors = 0;
static unsigned __CUnitTests_Global_quiet = 0;

static void __CUnitTests_printfQuiet(char *format, ...) {
	if (__CUnitTests_Global_quiet == 0) {
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
}

static char *__CUnitTests_getFileNameFromPath(char *filePath) {
	for (size_t i = strlen(filePath) - 1; i; i--) {
		if (filePath[i] == '/' || filePath[i] == '\\') {
			return &filePath[i + 1];
		}
	}
	return filePath;
}

static char *__CUnitTests_getTestResultString(__CUnitTests_Error result) {
	switch (result) {
		case __CUnitTests_Error_Succeed:
			return __CUnitTests_Global_outputColors ? "\033[0;32mSUCCEED\033[0m" : "SUCCEED";
		case __CUnitTests_Error_Failed: return __CUnitTests_Global_outputColors ? "\033[0;31mFAILED\033[0m" : "FAILED";
		case __CUnitTests_Error_NotExecuted:
			return __CUnitTests_Global_outputColors ? "\033[0;31mNOT_EXECUTED\033[0m" : "NOT_EXECUTED";
		default: return __CUnitTests_Global_outputColors ? "\033[0;31mERROR\033[0m" : "ERROR";
	}
}

static unsigned __CUnitTests_FilterTestsBySuiteName(__CUnitTests_Context *ctx, char **suites, unsigned suitesCount,
													__CUnitTests_Test **input, unsigned inputCount,
													__CUnitTests_Test ***output) {
	if (suitesCount == 0) {
		*output = input;
		return inputCount;
	}

	__CUnitTests_Test *temp[__CUNIT_TESTS_MAX];
	unsigned testsCount = 0;

	for (unsigned s = 0; s < suitesCount; s++) {
		char *suite = suites[s];
		unsigned found = 0;
		for (unsigned i = 0; i < inputCount; i++) {
			__CUnitTests_Test *test = input[i];
			if (test->suite != NULL && strcasecmp(test->suite, suite) == 0) {
				temp[testsCount] = test;
				testsCount++;
				found++;
				continue;
			}
		}

		if (!found) {
			printf("\nTest suite '%s' not found.", suite);
			ctx->executionResult = __CUnitTests_Error_Error;
		}
	}

	*output = temp;
	return testsCount;
}

static unsigned __CUnitTests_FilterTestsByTestName(__CUnitTests_Context *ctx, char **names, unsigned namesCount,
												   __CUnitTests_Test **input, unsigned inputCount,
												   __CUnitTests_Test ***output) {
	if (namesCount == 0) {
		*output = input;
		return inputCount;
	}

	__CUnitTests_Test *temp[__CUNIT_TESTS_MAX];
	unsigned testsCount = 0;

	for (unsigned n = 0; n < namesCount; n++) {
		char *name = names[n];
		unsigned found = 0;
		for (unsigned i = 0; i < inputCount; i++) {
			__CUnitTests_Test *test = input[i];
			if (strcasecmp(test->testName, name) == 0) {
				temp[testsCount] = test;
				testsCount++;
				found++;
				break;
			}
		}

		if (!found) {
			printf("\nTest '%s' not found.", name);
			ctx->executionResult = __CUnitTests_Error_Error;
		}
	}

	*output = temp;
	return testsCount;
}

static void __CUnitTests_FilterTests(__CUnitTests_Context *ctx, char **suites, unsigned suitesCount, char **names,
									 unsigned namesCount, __CUnitTests_Test **tests, unsigned testsCount) {
	if (suitesCount == 0 && namesCount == 0) {
		ctx->testsToExecute = tests;
		ctx->testsToExecuteCount = testsCount;
		return;
	}

	__CUnitTests_Test **bySuite = NULL;
	unsigned bySuiteCount = __CUnitTests_FilterTestsBySuiteName(ctx, suites, suitesCount, tests, testsCount, &bySuite);

	__CUnitTests_Test **byName = NULL;
	unsigned byNameCount = __CUnitTests_FilterTestsByTestName(ctx, names, namesCount, bySuite, bySuiteCount, &byName);

	ctx->testsToExecute = byName;
	ctx->testsToExecuteCount = byNameCount;
}

#define __CUnitTests_setContextAction(ctx, newAction)                                                                  \
	if (ctx->action == __CUnitTests_Action_PrintUsage) ctx->action = newAction

static __CUnitTests_Context *__CUnitTests_createContext(int argc, char *argv[]) {
	__CUnitTests_Context *ctx = malloc(sizeof(__CUnitTests_Context));
	ctx->executableName = argv[0];
	ctx->action = __CUnitTests_Action_PrintUsage;
	ctx->executionMode = __CUnitTests_ExecutionMode_InProcess;
	ctx->executionResult = __CUnitTests_Error_NotExecuted;

	char *testSuites[__CUNIT_TESTS_MAX_TEST_SUITES];
	unsigned testSuitesCount = 0;

	__CUnitTests_Global_outputColors = 0;
	__CUnitTests_Global_quiet = 0;

	int opt;
	while ((opt = getopt(argc, argv, "eiclqs:")) != -1) {
		switch (opt) {
			case 'l': __CUnitTests_setContextAction(ctx, __CUnitTests_Action_List); break;
			case 'c': __CUnitTests_Global_outputColors = 1; break;
			case 'e': __CUnitTests_setContextAction(ctx, __CUnitTests_Action_Execute); break;
			case 'i': ctx->executionMode = __CUnitTests_ExecutionMode_NewProcess; break;
			case 'q': __CUnitTests_Global_quiet = 1; break;
			case 's':
				if (testSuitesCount < __CUNIT_TESTS_MAX_TEST_SUITES) {
					char *suiteName = malloc(strlen(optarg));
					strcpy(suiteName, optarg);
					testSuites[testSuitesCount] = suiteName;
					testSuitesCount++;
				}
				break;
			default: break;
		}
	}

	int namesCount = argc - optind;
	char **names = &argv[optind];
	__CUnitTests_FilterTests(ctx, testSuites, testSuitesCount, names, namesCount, __CUnitTests_Global_tests,
							 __CUnitTests_Global_testsCount);
	return ctx;
}

static void __CUnitTests_printExecutingMessage(__CUnitTests_Test *test) {
	__CUnitTests_printfQuiet("\n'%s' started...", test->testName);
}

static void __CUnitTests_printTestResultMessage(__CUnitTests_Test *test) {
	char *testResultString = __CUnitTests_getTestResultString(test->result);
	__CUnitTests_printfQuiet("\n'%s' %s", test->testName, testResultString);
}

static void __CUnitTests_executeTestsInProcess(__CUnitTests_Context *ctx) {
	__CUnitTests_printfQuiet("Executing '%s' tests in process", ctx->executableName);

	for (unsigned int testIndex = 0; testIndex < ctx->testsToExecuteCount; testIndex++) {
		__CUnitTests_Test *test = ctx->testsToExecute[testIndex];
		if (test->result == __CUnitTests_Error_NotExecuted) {
			test->result = __CUnitTests_Error_Succeed;
			__CUnitTests_Global_currentTest = test;
			__CUnitTests_printExecutingMessage(test);
			if (test->setup != NULL) {
				test->setup();
			}
			test->routine();
			if (test->cleanup != NULL) {
				test->cleanup();
			}
			__CUnitTests_printTestResultMessage(test);
		}
	}
}

static __CUnitTests_Error __CUnitTests_getSeparateProcessResult(int closeResult) {
	switch (closeResult) {
		case 0: return __CUnitTests_Error_Succeed;
		case 2: return __CUnitTests_Error_Failed;
		case 3: return __CUnitTests_Error_NotExecuted;
		default: return __CUnitTests_Error_Error;
	}
}

static void __CUnitTests_executeTestsAsSeparateProcess(__CUnitTests_Context *ctx) {
	__CUnitTests_printfQuiet("Executing '%s' tests as separate processes", ctx->executableName);

	for (unsigned testIndex = 0; testIndex < ctx->testsToExecuteCount; testIndex++) {
		__CUnitTests_Test *test = ctx->testsToExecute[testIndex];
		if (test->result == __CUnitTests_Error_NotExecuted) {
			test->result = __CUnitTests_Error_Succeed;
			int test_command_size =
				snprintf(NULL, 0, __CUNIT_TESTS_RUN_COMMAND_PATTERN, ctx->executableName, test->testName) + 1;

			char *test_command = malloc(test_command_size);
			snprintf(test_command, test_command_size, __CUNIT_TESTS_RUN_COMMAND_PATTERN, ctx->executableName,
					 test->testName);
			__CUnitTests_printExecutingMessage(test);
			test->result = __CUnitTests_Error_Error;
			FILE *testProcess = popen(test_command, "r");
			if (testProcess != NULL) {
				char output[1000];
				while (fgets(output, sizeof(output), testProcess) != NULL) {
					printf("%s", output);
				}
				test->result = __CUnitTests_getSeparateProcessResult(pclose(testProcess));
			}
			free(test_command);
			__CUnitTests_printTestResultMessage(test);
		}
	}
}

static void __CUnitTests_getResults(__CUnitTests_Context *ctx) {
	unsigned tests_succeed = 0;
	unsigned tests_failed = 0;
	unsigned tests_errored = 0;

	for (unsigned testIndex = 0; testIndex < ctx->testsToExecuteCount; testIndex++) {
		__CUnitTests_Test *test = ctx->testsToExecute[testIndex];
		switch (test->result) {
			case __CUnitTests_Error_Succeed: tests_succeed++; break;
			case __CUnitTests_Error_Failed: tests_failed++; break;
			default: tests_errored++; break;
		}
	}

	if (__CUnitTests_Global_quiet == 0 && (tests_failed || tests_errored)) {
		printf("\n%s tests execution failures:", __CUnitTests_getFileNameFromPath(ctx->executableName));
		for (unsigned testIndex = 0; testIndex < ctx->testsToExecuteCount; testIndex++) {
			__CUnitTests_Test *test = ctx->testsToExecute[testIndex];
			if (test->result != __CUnitTests_Error_Succeed) {
				printf("\n%s:\t%s", test->testName, __CUnitTests_getTestResultString(test->result));
			}
		}
	}

	if (tests_errored > 0) {
		ctx->executionResult = __CUnitTests_Error_Error;
	} else if (tests_failed > 0) {
		ctx->executionResult = __CUnitTests_Error_Failed;
	} else {
		ctx->executionResult = __CUnitTests_Error_Succeed;
	}

	if (__CUnitTests_Global_quiet == 0) {
		char *testsResultString = __CUnitTests_getTestResultString(ctx->executionResult);
		printf("\n'%s' result: %s. Total: %u. Succeed: %u. Failed: %u. Errors: %u\n", ctx->executableName,
			   testsResultString, ctx->testsToExecuteCount, tests_succeed, tests_failed, tests_errored);
	}
}

static void __CUnitTests_executeTests(__CUnitTests_Context *ctx) {
	if (ctx->executionResult == __CUnitTests_Error_NotExecuted) {
		if (ctx->executionMode == __CUnitTests_ExecutionMode_InProcess) {
			__CUnitTests_executeTestsInProcess(ctx);
		} else {
			__CUnitTests_executeTestsAsSeparateProcess(ctx);
		}
	}
	__CUnitTests_getResults(ctx);
}

static void __CUnitTests_printUsage(__CUnitTests_Context *ctx) {
	char *executableName = __CUnitTests_getFileNameFromPath(ctx->executableName);
	printf("Usage:\n");
	printf("%s -e                       - execute all tests\n", executableName);
	printf("%s -ei                      - execute all tests as separate processes\n", executableName);
	printf("%s -e first second          - execute selected tests\n", executableName);
	printf("%s -ei first second         - execute selected tests as separate processes\n", executableName);
	printf("%s -e -s first -s second    - execute selected test suites\n", executableName);
	printf("%s -l                       - list all tests\n", executableName);
	printf("%s                          - print usage\n", executableName);
	printf("\nAdditional flags:\n");
	printf("-c                          - color output\n");
	printf("-q                          - quiet mode (no tests summaries)\n");
	ctx->executionResult = __CUnitTests_Error_Succeed;
}

static void __CUnitTests_listTests(__CUnitTests_Context *ctx) {
	for (unsigned index = 0; index < __CUnitTests_Global_testsCount; index++) {
		printf("%s\n", __CUnitTests_Global_tests[index]->testName);
	}
	ctx->executionResult = __CUnitTests_Error_Succeed;
}

static __CUnitTests_Error __CUnitTests_performAction(__CUnitTests_Context *ctx) {
	switch (ctx->action) {
		case __CUnitTests_Action_List: __CUnitTests_listTests(ctx); break;
		case __CUnitTests_Action_Execute: __CUnitTests_executeTests(ctx); break;
		default: __CUnitTests_printUsage(ctx); break;
	}
	return ctx->executionResult;
}

int main(int argc, char *argv[]) {
	__CUnitTests_Context *ctx = __CUnitTests_createContext(argc, argv);
	__CUnitTests_Error result = __CUnitTests_performAction(ctx);
	return result;
}

static void __CUnitTests_setTestSucceed(char *file, int line) {
	printf("\n\tChanging test result to SUCCEED in in %s:%d", file, line);
	__CUnitTests_Global_currentTest->result = __CUnitTests_Error_Succeed;
}

static void __CUnitTests_setTestFailed(char *file, int line) {
	printf("\n\tChanging test result to FAILED in %s:%d", file, line);
	__CUnitTests_Global_currentTest->result = __CUnitTests_Error_Failed;
}

static void __CUnitTests_assertionFailed(char *file, int line, char *message, ...) {
	__CUnitTests_Global_currentTest->result = __CUnitTests_Error_Failed;
	va_list args;
	va_start(args, message);
	printf("\n\tAssertion failed: '");
	vprintf(message, args);
	printf("' in %s:%d", file, line);
	va_end(args);
}

#define test(name, ...)                                                                                                \
	void ___CUnitTests_test_routine_##name();                                                                          \
	static __CUnitTests_Test __CUnitTests_Test_##name = {.testName = #name,                                            \
														 .routine = &___CUnitTests_test_routine_##name,                \
														 .result = __CUnitTests_Error_NotExecuted,                     \
														 __VA_ARGS__};                                                 \
	__attribute__((constructor)) void ___CUnitTests_register_test_##name() {                                           \
		int id = __COUNTER__;                                                                                          \
		__CUnitTests_Global_testsCount++;                                                                              \
		__CUnitTests_Global_tests[id] = &__CUnitTests_Test_##name;                                                     \
	}                                                                                                                  \
	void ___CUnitTests_test_routine_##name()

#define test_set_failed() __CUnitTests_setTestFailed(__FILE__, __LINE__)
#define test_set_succeed() __CUnitTests_setTestSucceed(__FILE__, __LINE__)
#define test_failed() __CUnitTests_Global_currentTest->result == __CUnitTests_Error_Failed

#define test_assert_true(expr)                                                                                         \
	if (!(expr)) __CUnitTests_assertionFailed(__FILE__, __LINE__, #expr)

#define test_assert_true_fmt(expr, message, ...)                                                                       \
	if (!(expr)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_false(expr)                                                                                        \
	if ((expr)) __CUnitTests_assertionFailed(__FILE__, __LINE__, #expr)

#define test_assert_false_fmt(expr, message, ...)                                                                      \
	if ((expr)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_equal(expected, result)                                                                            \
	if ((expected) != (result)) __CUnitTests_assertionFailed(__FILE__, __LINE__, "%s==%s", #expected, #result);

#define test_assert_equal_fmt(expected, result, message, ...)                                                          \
	if ((expected) != (result)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__);

#define test_assert_not_equal(expected, result)                                                                        \
	if ((expected) == (result)) __CUnitTests_assertionFailed(__FILE__, __LINE__, "%s!=%s", #expected, #result)

#define test_assert_not_equal_fmt(expected, result, message, ...)                                                      \
	if ((expected) == (result)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_null(value)                                                                                        \
	if ((value) != (NULL)) __CUnitTests_assertionFailed(__FILE__, __LINE__, "%s == NULL", #value)

#define test_assert_null_fmt(value, message, ...)                                                                      \
	if ((value) != (NULL)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_not_null(value)                                                                                    \
	if ((value) == (NULL)) __CUnitTests_assertionFailed(__FILE__, __LINE__, "%s != NULL", #value)

#define test_assert_not_null_fmt(value, message, ...)                                                                  \
	if ((value) == (NULL)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#endif /* _CUnitTests_CUnitTests_h_ */
