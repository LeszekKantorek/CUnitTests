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

#include "CUnitTests/CUnitTests.h"

static unsigned someValue = 0;
static unsigned setup1_called = 0;
static unsigned setup2_called = 0;
static unsigned cleanup2_called = 0;

static void setup1() {
	someValue = 345;
	setup1_called = 1;
}

test(setup1, .setup = &setup1) {
	test_assert_equal(345, someValue);
}

static void setup2() {
	someValue = 123;
	setup2_called = 1;
}

static void cleanup2() {
	cleanup2_called = 1;
}

test(setup2_cleanup2, .setup = &setup2, .cleanup = &cleanup2) {
	test_assert_equal(123, someValue);
}

test(verify_setup_cleanup) {
	test_assert_true(setup1_called);
	test_assert_true(setup2_called);
	test_assert_true(cleanup2_called);
}
