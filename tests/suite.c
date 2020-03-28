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

unsigned suite1 = 0; 
unsigned suite2 = 0;
unsigned other = 0;

test(suite1_1, .suite = "suite1") {
	suite1++;
}

test(suite1_2, .suite = "suite1") {
	suite1++;
}

test(suite2, .suite = "suite2") {
	suite2++;
}

static void setup() {}

test(suite2_setup, .suite = "suite2", .setup = &setup) {
	suite2++;
}

test(someOtherTest) {
	other++;
}

test(verify_suite, .suite = "suite1") {
	test_assert_true(suite1);
	test_assert_false(suite2);
	test_assert_false(other);
}
