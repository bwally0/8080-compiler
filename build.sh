#!/bin/bash

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

cmake ..
cmake --build .

if [ -f "./bmlc" ]; then
    (cd .. && ./build/bmlc)
elif [ -f "./bmlc.exe" ]; then
    (cd .. && ./build/bmlc.exe)
else
    echo "Error: Executable not found"
    exit 1
fi