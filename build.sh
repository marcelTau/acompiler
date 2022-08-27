#!/bin/bash

cmake -S . -B build 
cmake --build build 
[[ $? -ne 0 ]] && echo "COMPILATION FAILED ..." && exit 1
./build/acompiler today
cat ./testoutput.asm

nasm -felf64 testoutput.asm && ld testoutput.o -o testoutput && ./testoutput; echo $?

#cd build 
#ctest --output-on-failure -j 4
#cd ..
