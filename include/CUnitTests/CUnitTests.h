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
#define __CUNIT_TESTS_RUN_COMMAND_PATTERN "%s -e %s >nul"

typedef enum __CUnitTests_Error {
	__CUnitTests_Error_Succeed = 0,
	__CUnitTests_Error_Error = 1,
	__CUnitTests_Error_Failed = 2,
	__CUnitTests_Error_NotFound = 3,
	__CUnitTests_Error_NotExecuted = 4,
} __CUnitTests_Error;

typedef struct __CUnitTests_Test {
	char *test_name;
	void (*test_routine)();
	__CUnitTests_Error result;
} __CUnitTests_Test;

typedef enum __CUnitTests_Action { __CUnitTests_Action_List = 0, __CUnitTests_Action_Execute = 1 } __CUnitTests_Action;

typedef enum __CUnitTests_ExecutionMode {
	__CUnitTests_ExecutionMode_InProcess = 0,
	__CUnitTests_ExecutionMode_NewProcess = 1
} __CUnitTests_ExecutionMode;

typedef struct __CUnitTests_Context {
	char *executableName;
	__CUnitTests_Action action;
	__CUnitTests_ExecutionMode executionMode;
	__CUnitTests_Error executionResult;
	__CUnitTests_Test *testsToExecute;
	unsigned testsToExecuteCount;
} __CUnitTests_Context;

static __CUnitTests_Test __CUnitTests_Global_tests[__CUNIT_TESTS_MAX];
static unsigned __CUnitTests_Global_testsCount = 0;
static __CUnitTests_Test *__CUnitTests_Global_currentTest = NULL;

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
			return "SUCCEED";
		case __CUnitTests_Error_Failed:
			return "FAILED";
			break;
		case __CUnitTests_Error_NotFound:
			return "NOT FOUND";
			break;
		case __CUnitTests_Error_NotExecuted:
			return "NOT EXECUTED";
			break;
		default:
			break;
	}

	return "ERROR";
}

static void __CUnitTests_findTests(__CUnitTests_Context *ctx, char **specifiedTestsNames, int specifiedTestsCount) {
	__CUnitTests_Test *specifiedTests = malloc(specifiedTestsCount * sizeof(__CUnitTests_Test));
	for (int specifiedIndex = 0; specifiedIndex < specifiedTestsCount; specifiedIndex++) {
		char *searchedTestName = specifiedTestsNames[specifiedIndex];
		__CUnitTests_Test *searchedTest = NULL;
		for (unsigned testIndex = 0; testIndex < __CUnitTests_Global_testsCount; testIndex++) {
			if (strcasecmp(__CUnitTests_Global_tests[testIndex].test_name, searchedTestName) == 0) {
				searchedTest = &__CUnitTests_Global_tests[testIndex];
				break;
			}
		}
		if (searchedTest == NULL) {
			searchedTest = malloc(sizeof(__CUnitTests_Test));
			searchedTest->test_name = searchedTestName;
			searchedTest->test_routine = NULL;
			searchedTest->result = __CUnitTests_Error_NotFound;
		}
		specifiedTests[specifiedIndex] = *searchedTest;
	}
	ctx->testsToExecute = specifiedTests;
	ctx->testsToExecuteCount = specifiedTestsCount;
}

static __CUnitTests_Context *__CUnitTests_createContext(int argc, char *argv[]) {
	__CUnitTests_Context *ctx = malloc(sizeof(__CUnitTests_Context));
	ctx->executableName = argv[0];
	ctx->action = __CUnitTests_Action_List;
	ctx->executionMode = __CUnitTests_ExecutionMode_InProcess;
	ctx->executionResult = __CUnitTests_Error_NotExecuted;

	int opt;
	while ((opt = getopt(argc, argv, "ei")) != -1) {
		switch (opt) {
			case 'e':
				ctx->action = __CUnitTests_Action_Execute;
				break;
			case 'i':
				ctx->executionMode = __CUnitTests_ExecutionMode_NewProcess;
				break;
			default:
				break;
		}
	}

	int specifiedTestsCount = argc - optind;
	if (specifiedTestsCount > 0) {
		char **specifiedTestsNames = &argv[optind];
		__CUnitTests_findTests(ctx, specifiedTestsNames, specifiedTestsCount);
	} else {
		ctx->testsToExecute = __CUnitTests_Global_tests;
		ctx->testsToExecuteCount = __CUnitTests_Global_testsCount;
	}

	return ctx;
}

static void __CUnitTests_printError(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

static void __CUnitTests_printOut(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
}


static void __CUnitTests_printExecutingMessage(__CUnitTests_Test *test) {
	__CUnitTests_printOut("\n'%s' started...\t", test->test_name);
}

static void __CUnitTests_printTestResultMessage(__CUnitTests_Test *test) {
	char *testResultString = __CUnitTests_getTestResultString(test->result);
	if (test->result == __CUnitTests_Error_Succeed) {
		__CUnitTests_printOut("\n'%s' %s", test->test_name, testResultString);
	} else {
		__CUnitTests_printError("\n'%s' %s", test->test_name, testResultString);
	}
}

static void __CUnitTests_executeTestsInProcess(__CUnitTests_Context *ctx) {
	for (unsigned testIndex = 0; testIndex < ctx->testsToExecuteCount; testIndex++) {
		__CUnitTests_Test *test = &ctx->testsToExecute[testIndex];
		if (test->result == __CUnitTests_Error_NotExecuted) {
			test->result = __CUnitTests_Error_Succeed;
			__CUnitTests_Global_currentTest = test;
			__CUnitTests_printExecutingMessage(test);
			test->test_routine();
			__CUnitTests_printTestResultMessage(test);
		}
	}
}

static void __CUnitTests_executeTestsAsSeparateProcess(__CUnitTests_Context *ctx) {
	for (unsigned testIndex = 0; testIndex < ctx->testsToExecuteCount; testIndex++) {
		__CUnitTests_Test *test = &ctx->testsToExecute[testIndex];
		if (test->result == __CUnitTests_Error_NotExecuted) {
			test->result = __CUnitTests_Error_Succeed;
			int test_command_size =
				snprintf(NULL, 0, __CUNIT_TESTS_RUN_COMMAND_PATTERN, ctx->executableName, test->test_name) + 1;

			char *test_command = malloc(test_command_size);
			snprintf(test_command, test_command_size, __CUNIT_TESTS_RUN_COMMAND_PATTERN, ctx->executableName,
					 test->test_name);
			__CUnitTests_printExecutingMessage(test);
			test->result = system(test_command);
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
		__CUnitTests_Test *test = &ctx->testsToExecute[testIndex];
		switch (test->result) {
			case __CUnitTests_Error_Succeed:
				tests_succeed++;
				break;
			case __CUnitTests_Error_Failed:
				tests_failed++;
				break;
			default:
				tests_errored++;
				break;
		}
	}

	if (tests_failed || tests_errored) {
		__CUnitTests_printError("\n\n%s tests execution failures:", __CUnitTests_getFileNameFromPath(ctx->executableName));
		for (unsigned testIndex = 0; testIndex < ctx->testsToExecuteCount; testIndex++) {
			__CUnitTests_Test *test = &ctx->testsToExecute[testIndex];
			if (test->result != __CUnitTests_Error_Succeed) {
				__CUnitTests_printError("\n%s:\t%s", test->test_name, __CUnitTests_getTestResultString(test->result));
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

	if (ctx->executionResult == __CUnitTests_Error_Succeed) {
		__CUnitTests_printOut("\n\nTotal: %u. Succeed: %u. Failed: %u. Errors: %u \n", ctx->testsToExecuteCount, tests_succeed,
			   tests_failed, tests_errored);
	}else{
		__CUnitTests_printError( "\n\nTotal: %u. Succeed: %u. Failed: %u. Errors: %u \n", ctx->testsToExecuteCount, tests_succeed,
			   tests_failed, tests_errored);
	}
}

static void __CUnitTests_executeTests(__CUnitTests_Context *ctx) {
	if (ctx->executionMode == __CUnitTests_ExecutionMode_InProcess) {
		__CUnitTests_executeTestsInProcess(ctx);
	} else {
		__CUnitTests_executeTestsAsSeparateProcess(ctx);
	}

	__CUnitTests_getResults(ctx);
}

static void __CUnitTests_listTests(__CUnitTests_Context *ctx) {
	char *executableName = __CUnitTests_getFileNameFromPath(ctx->executableName);

	__CUnitTests_printOut( "\n%s usage:", executableName);
	__CUnitTests_printOut( "\n%s -e                       - execute all tests", executableName);
	__CUnitTests_printOut( "\n%s -ei                      - execute all tests in isolation", executableName);
	__CUnitTests_printOut( "\n%s -e first second ...      - execute selected tests", executableName);
	__CUnitTests_printOut( "\n%s -ei first second ...     - execute selected tests in isolation", executableName);
	__CUnitTests_printOut( "\n%s                          - list all tests", executableName);

	__CUnitTests_printOut("\nAvailable tests:\n");
	for (unsigned index = 0; index < __CUnitTests_Global_testsCount; index++) {
		__CUnitTests_printOut("\n%s", __CUnitTests_Global_tests[index].test_name);
	}

	ctx->executionResult = __CUnitTests_Error_Succeed;
}

static __CUnitTests_Error __CUnitTests_performAction(__CUnitTests_Context *ctx) {
	switch (ctx->action) {
		case __CUnitTests_Action_Execute:
			__CUnitTests_executeTests(ctx);
			break;
		default:
			__CUnitTests_listTests(ctx);
			break;
	}

	return ctx->executionResult;
}

int main(int argc, char *argv[]) {
	__CUnitTests_Context *ctx = __CUnitTests_createContext(argc, argv);
	__CUnitTests_Error result = __CUnitTests_performAction(ctx);
	return result;
}

static void __CUnitTests_setTestSucceed() { __CUnitTests_Global_currentTest->result = __CUnitTests_Error_Succeed; }

static void __CUnitTests_setTestFailed() { __CUnitTests_Global_currentTest->result = __CUnitTests_Error_Failed; }

static void __CUnitTests_assertionFailed(char *file, int line, char *message, ...) {
	__CUnitTests_setTestFailed();
	va_list args;
	va_start(args, message);
	fprintf(stderr,"\n\tAssertion failed: '");
	vfprintf(stderr, message, args);
	fprintf(stderr, "' in %s:%d", file, line);
	va_end(args);
}

#define test(name, ...)                                                                                                \
	void ___CUnitTests_test_routine_##name() { __VA_ARGS__ }                                                           \
	__attribute__((constructor)) void ___CUnitTests_register_test_##name() {                                           \
		int id = __COUNTER__;                                                                                          \
		__CUnitTests_Global_testsCount++;                                                                              \
		__CUnitTests_Global_tests[id].test_name = #name;                                                               \
		__CUnitTests_Global_tests[id].test_routine = &___CUnitTests_test_routine_##name;                               \
		__CUnitTests_Global_tests[id].result = __CUnitTests_Error_NotExecuted;                                         \
	}

#define test_set_failed() __CUnitTests_setTestFailed()
#define test_set_succeed() __CUnitTests_setTestSucceed()
#define test_failed() __CUnitTests_Global_currentTest->result == __CUnitTests_Error_Failed

#define test_assert_true(expr, message, ...)                                                                           \
	if (!(expr)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_false(expr, message, ...)                                                                          \
	if ((expr)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_equal(expected, result, message, ...)                                                              \
	if ((expected) != (result)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__);

#define test_assert_not_equal(expected, result, message, ...)                                                          \
	if ((expected) == (result)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_null(value, message, ...)                                                                          \
	if ((value) != (NULL)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#define test_assert_not_null(value, message, ...)                                                                      \
	if ((value) == (NULL)) __CUnitTests_assertionFailed(__FILE__, __LINE__, message, ##__VA_ARGS__)

#endif /* _CUnitTests_CUnitTests_h_ */
