#include "cfakes/cfakes.h"

void _status_should_be_unknown(){
    if(_cfakes_context.current_test_result!=CFAKES_TEST_RESULT_UNKNOWN){
       _cfakes_set_test_failed();
    }
}

void _status_should_be_failed(){
    if(_cfakes_context.current_test_result==CFAKES_TEST_RESULT_FAILED){
        _cfakes_set_test_succeed();
    }else{
        _cfakes_set_test_failed();
    }
}

void assert_true_succeed_test(){
    cfakes_assert_true(1, "Should be true");
    _status_should_be_unknown();
}

void assert_true_failed_test(){
    cfakes_assert_true(0, "Should be true");
   _status_should_be_failed();
}

void assert_false_succeed_test(){
    cfakes_assert_false(0, "Should be false");
     _status_should_be_unknown();
}

void assert_false_failed_test(){
    cfakes_assert_false(1, "Should be false");
    _status_should_be_failed();
}

void assert_equal_succeed_test(){
    int expected = 1;
    int actual = 1;
    
    cfakes_assert_equal(expected, actual, "Should be %d but was %d", expected, actual);
    _status_should_be_unknown();
}

void assert_equal_failed_test(){
    int expected = 2;
    int actual = 1;
    
    cfakes_assert_equal(expected, actual, "Should be %d but was %d", expected, actual);
     _status_should_be_failed();
}

void assert_not_equal_succeed_test(){
    int expected = 2;
    int actual = 1;
    
    cfakes_assert_not_equal(expected, actual, "Should not be %d but was %d", expected, actual);
     _status_should_be_unknown();
}

void assert_not_equal_failed_test(){
    int expected = 2;
    int actual = 2;
    
    cfakes_assert_not_equal(expected, actual, "Should not be %d but was %d", expected, actual);
     _status_should_be_failed();
}

void assert_null_succeed_test(){
    char *test = NULL;
    cfakes_assert_null(test, "Should be null");
     _status_should_be_unknown();
}

void assert_null_failed_test(){
    char *test = "Some string";
    cfakes_assert_null(test, "Should be null");
    _status_should_be_failed();
}

void assert_not_null_succeed_test(){
    char *test = "Some string";
    cfakes_assert_not_null(test, "Should not be null");
     _status_should_be_unknown();
}

void assert_not_null_failed_test(){
      char *test = NULL;
    cfakes_assert_not_null(test, "Should be null");
    _status_should_be_failed();
}