
#include "interaction.h"



int main(int argv, char **args) {
    if (argv < 3) {
        printf("Not enough arguments\n");
        return 1;
    }

    if (argv > 3) {
        printf("Too many args\n");
        return 1;
    }

    if (strcmp(args[1], "-p") != 0) {
        printf("Invalid argument, god: %s, should be: -p\n", args[1]);
        return 1;
    }

    #ifdef STEPS
        printf("Prepare args\n");
    #endif
    
    // n = x + 1
    int n = strtol(args[2], NULL, 10);
    n = n + 1;

    FILE *events = fopen(events_log, "w");
    FILE *pipes = fopen(pipes_log, "w");


    printf("STEP 1: manager\n");
    Manager *manager = newManager(n);

    printf("STEP 2: pipes\n");
    int index = 0;
    for (int i = 0; i < manager->pidLen; i++) {
        for (int j = 0; j < manager->pidLen; j++) {
            
            if (i == j) {
                continue;
            }

            Channel *chan = newChannel(i, j);
            if (pipe(chan->PipesFd) == -1) {
                perror("pipe");
                return -1;
            }
            manager->chan[index++] = *chan;

            fprintf(pipes, "Pipe from %d to %d \t read: %d write: %d is created!\n", i, j, chan->PipesFd[0], chan->PipesFd[1]);
            fflush(pipes);
        }
    }

    printf("STEP 3: pids\n");
    for (int i = 1; i < manager->pidLen; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            manager->pid[i].pid = getpid();

            // Делаю работу в процессе 
            int res = interaction(manager, events, pipes, i);

            // Проверяю результат работы процесса
            if (res == -1)
                exit(EXIT_SUCCESS);
            else {
                exit(EXIT_FAILURE);
            }
            
        }else {
            manager->pid[0].pid = getpid();
        }
    }
    sleep(1);
    wait(NULL);  

    printf("STEP 4: message\n");
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, MAX_PAYLOAD_LEN, STARTED, time(NULL));
    char buffer[MAX_PAYLOAD_LEN];
    sprintf(buffer, log_started_fmt, 0, getpid(), getppid());
    Message *msg = newMessage(header, buffer); 


    // Получаю ото всех сообщения START
    printf("STEP 5: read msg START\n");
    for (int i = 1; i < manager->pidLen; i++) {

        int res = receive(manager, i, msg);
        int proc = 0;
        if (res == -1) {
            printf("Faild to get message from: %d, to: %d\n", i, proc);
            return -1;
        }

        printf("%s", msg->s_payload);

        fprintf(events, "%s", msg->s_payload);
        fflush(events);

        mnemonic_pipe(pipes, i, proc, proc, msg->s_payload);
        fflush(pipes);
    }


    // Получаю ото всех сообщения DONE
    printf("STEP 6: read msg DONE\n");
    for (int i = 1; i < manager->pidLen; i++) {

        int res = receive(manager, i, msg);
        int proc = 0;
        if (res == -1) {
            printf("Faild to get message from: %d, to: %d\n", i, proc);
            return -1;
        }

        printf("%s", msg->s_payload);

        fprintf(events, "%s", msg->s_payload);
        fflush(events);

        mnemonic_pipe(pipes, i, proc, proc, msg->s_payload);
        fflush(pipes);
    }

    
    // Закрываю пайп
    printf("STEP 8: close\n");
    for (int i = 0; i < manager->chanLen; i++) {
        close(manager->chan[i].PipesFd[0]);
        close(manager->chan[i].PipesFd[1]);
        fprintf(pipes, "Pipe from %d to %d \t read: %d write: %d is closed!\n", 
        manager->chan[i].FromTo[0], manager->chan[i].FromTo[1], manager->chan[i].PipesFd[0], manager->chan[i].PipesFd[1]);
        fflush(pipes);
    }



    fclose(events);
    fclose(pipes);

    return 0;
}