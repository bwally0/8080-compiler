#!/bin/bash

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

cmake ..
cmake --build .

cd ..

EXECUTABLE="./build/bmlc"
if [ ! -f "$EXECUTABLE" ] && [ -f "./build/bmlc.exe" ]; then
    EXECUTABLE="./build/bmlc.exe"
fi

if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Executable not found"
    exit 1
fi

# Test file can be passed as argument, defaults to fib.bml
TEST_FILE="${1:-./tests/fib.bml}"
$EXECUTABLE "$TEST_FILE"