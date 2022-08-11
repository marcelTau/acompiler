#!/bin/bash

cmake -S . -B build 
cmake --build build 
cd build 
ctest --output-on-failure -j 4
cd ..
