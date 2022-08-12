#!/bin/bash

cmake -S . -B build 
cmake --build build 
[[ $? -ne 0 ]] && echo "COMPILATION FAILED ..." && exit 1
cd build 
ctest --output-on-failure -j 4
cd ..
