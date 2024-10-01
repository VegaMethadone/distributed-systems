rm main
rm events.log
rm pipes.log


clang -o main -std=c99 -Wall -pedantic *.c

./main -p 3