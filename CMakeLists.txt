cmake_minimum_required(VERSION 3.11)
project(CUnitTests VERSION 1.3 LANGUAGES C)

if(CUNITTESTS_FLAGS)
	string(APPEND CMAKE_C_FLAGS " -Werror -Wall -Wextra ")
	string(APPEND CMAKE_CXX_FLAGS " -Werror -Wall -Wextra ")

	string(APPEND CMAKE_C_FLAGS " -Wno-unused-function ")
	string(APPEND CMAKE_CXX_FLAGS " -Wno-unused-function ")
	
	string(APPEND CMAKE_C_FLAGS " -Wno-gnu-zero-variadic-macro-arguments ")
	string(APPEND CMAKE_CXX_FLAGS " -Wno-gnu-zero-variadic-macro-arguments ")
endif()

add_library(CUnitTests INTERFACE)
target_include_directories(CUnitTests INTERFACE include/)

if(CUNITTESTS_TESTING)
	add_executable(CUnitTestsExample tests/example.c)
	target_link_libraries(CUnitTestsExample CUnitTests)

	add_executable(CUnitTestsSetupCleanup tests/setup_cleanup.c)
	target_link_libraries(CUnitTestsSetupCleanup CUnitTests)

	add_executable(CUnitTestsSuite tests/suite.c)
	target_link_libraries(CUnitTestsSuite CUnitTests)

	enable_testing()
	add_test(NAME CUnitTestsExampleInProcess COMMAND CUnitTestsExample -e)
	add_test(NAME CUnitTestsExampleAsSeparate COMMAND CUnitTestsExample -ei)
	add_test(NAME CUnitTestsSetupCleanup COMMAND CUnitTestsSetupCleanup -e)
	add_test(NAME CUnitTestsSuite COMMAND CUnitTestsSuite -e -s suite1)
	
endif()