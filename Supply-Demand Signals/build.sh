#!/bin/bash

mkdir -p build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. || exit 1
cmake --build . || exit 1
