#!/bin/bash


rm main
rm events.log
rm pipes.log


export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$(pwd)/lib64"


clang -o main -g -std=c99 -Wall -pedantic *.c -L./lib64 -lruntime

LD_PRELOAD=$(pwd)/lib64/libruntime.so ./main --mutexl -p 9

#./main -p 3 10 20 30
