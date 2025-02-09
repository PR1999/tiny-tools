#!/usr/bin/bash
FILE=$1
echo Compiling $FILE
gcc -nostdlib -static -o ${FILE}.elf ${FILE}.s
echo Dumping $FILE
objcopy --dump-section .text=${FILE}.bin ${FILE}.elf
gcc -nostdlib -static -g -o db${FILE}.elf ${FILE}.s
echo debug elf made
