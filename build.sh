#!/bin/bash

cmake -S . -B build 
cmake --build build 
[[ $? -ne 0 ]] && echo "COMPILATION FAILED ..." && exit 1
./build/acompiler today

echo "==================== LOX ===================="
cat today
echo "==================== ASM ===================="
cat ./testoutput.asm

nasm -felf64 testoutput.asm && ld testoutput.o -o testoutput && ./testoutput
echo "return value: $?"

#cd build 
#ctest --output-on-failure -j 4
#cd ..
