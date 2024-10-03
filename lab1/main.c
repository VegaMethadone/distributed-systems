
#include "interaction.h"

#define STEPS


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

    #ifdef STEPS
        printf("STEP 1: manager\n");
    #endif
    Manager *manager = newManager(n);

    #ifdef STEPS
        printf("STEP 2: pipes\n");
    #endif
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


    #ifdef STEPS
        printf("STEP 3: pids\n");
    #endif
    for (int i = 1; i < manager->pidLen; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            manager->pid[i].pid = getpid();

            // Закрываю неиспользуемые дескриптеры в child-process
            closeNotUsableFD(manager, i, pipes);

            // Делаю работу в процессе 
            int res = interaction(manager, events, pipes, i);

            // Проверяю результат работы процесса
            if (res == -1) {
                printf("proc: %d faild!\n", i);
                exit(EXIT_FAILURE);
            }
            else {
                printf("proc: %d is done!\n", i);
                exit(EXIT_SUCCESS);
            }
            
        }

    }

    // Настраиваю родительский процесс
    manager->pid[0].pid = getpid();
    closeNotUsableFD(manager, 0, pipes);

    // Также закрываю пайпы на запись в родительском процессе т.к. он только мониторит
    /*
        Если тест выдаст ошибку - сделать
    */
    /*
        sleep(1);
        wait(NULL);  
    */

    int status;
    while (wait(&status) > 0) {
        if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("Child terminated abnormally\n");
        }
    }



    #ifdef STEPS
        printf("STEP 4: message\n");
    #endif
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, 48, STARTED, time(NULL));
    char buffer[MAX_PAYLOAD_LEN];
    sprintf(buffer, log_started_fmt, 0, getpid(), getppid());
    Message *msg = newMessage(header, buffer); 


    // Получаю ото всех сообщения START
    #ifdef STEPS
        printf("STEP 5: read msg START\n");
    #endif
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


    #ifdef STEPS
        printf("STEP 6: read msg DONE\n");
    #endif
    header = newMessageHeader(MESSAGE_MAGIC, 28, DONE, time(NULL));
    char newBuffer[MAX_PAYLOAD_LEN];
    msg = newMessage(header, newBuffer);

    // Получаю ото всех сообщения DONE
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

    fprintf(events, log_received_all_done_fmt, 0);
    fflush(events);
    
    // Закрываю пайп
    #ifdef STEPS
        printf("STEP 8: close\n");
    #endif
    closeUsableFD(manager, 0, pipes);


    fclose(events);
    fclose(pipes);

    return 0;
}
