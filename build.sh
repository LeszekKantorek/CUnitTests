#!/bin/bash

function clean(){
    rm -rf $BUILD_DIR
}

function code(){
    cmake -GXcode -B$BUILD_DIR .
}

function compile(){
    code
    cmake --build $BUILD_DIR --target ALL_BUILD --config $CONFIG
}

function run_tests(){
    compile
    cd $BUILD_DIR/tests/
    ctest -C $CONFIG -V
    cd ../../
}

BUILD_DIR=_build
COMMAND=$1
CONFIG=$2

if [ -z "$COMMAND"]; then COMMAND=code; fi
if [ -z "$CONFIG"]; then CONFIG=debug; fi

echo Executing build script: Command:$COMMAND Configuration:$CONFIG Architecture:$ARCHITECTURE

case $COMMAND in
"clean")
    clean
    ;;
"code")
    code
    ;;
"compile")
    compile
    ;;
"test")
    run_tests
    ;;
*)
    echo Unknown command: $COMMAND
    exit 1
    ;;
esac
