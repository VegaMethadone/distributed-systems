rm main
rm events.log
rm pipes.log


clang main.c msg.c interaction.c channel.c ipc.c -o main

./main -p 2