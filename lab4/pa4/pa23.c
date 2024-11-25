#include "banking.h"
#include "interaction.h"


#define STEPS



int main(int argv, char **args) {

    if (argv < 3) {
        printf("Not enough arguments\n");
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
    for (int i = 0; i < manager->pidLen; i++) {
        manager->pid[i] = *newProcess(i);
    }

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

            // на чтение
            int flags = fcntl(chan->PipesFd[0], F_GETFL, 0);
            fcntl(chan->PipesFd[0], F_SETFL, flags | O_NONBLOCK);

            // на запись
            flags = fcntl(chan->PipesFd[1], F_GETFL, 0);
            fcntl(chan->PipesFd[1], F_SETFL, flags | O_NONBLOCK);


            manager->chan[index++] = *chan;

            fprintf(pipes, "Pipe from %d to %d \t read: %d write: %d is created!\n", i, j, chan->PipesFd[0], chan->PipesFd[1]);
            fflush(pipes);
        }
    } 


    #ifdef STEPS
        printf("STEP 3: pids\n");
    #endif
    for (int i = 1; i < manager->pidLen; i++) {
        // added
        // sleep(1);
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            manager->pid[i].pid = getpid();

            // Закрываю неиспользуемые дескриптеры в child-process
            closeNotUsableFD(manager, i, pipes);


            // Выполнение работы дочерним процессом
            int res = interaction(manager, events, pipes, i);

            if (res == -1) {
                printf("proc: %d faild!\n", i);
                exit(EXIT_FAILURE);
            }
            else {
                exit(EXIT_SUCCESS);
            }
            
        }

    }

    // Настраиваю родительский процесс
    // Также закрываю пайпы на запись в родительском процессе т.к. он только мониторит
    manager->pid[0].pid = getpid();
    closeNotUsableFD(manager, 0, pipes);



    #ifdef STEPS
        printf("STEP 4: message\n");
    #endif
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, strlen(log_started_fmt), STARTED, get_lamport_time());
    char buffer[MAX_PAYLOAD_LEN];
    sprintf(buffer, log_started_fmt, get_lamport_time(), 0, getpid(), getppid(), 0);
    Message *msg = newMessage(header, buffer); 


    // Получаю ото всех сообщения START
    #ifdef STEPS
        printf("STEP 5: read msg START\n");
    #endif
    for (int i = 1; i < manager->pidLen; i++) {
        
        int res = 1;
        while ((res = receive(manager, i, msg)) != 0 ) {
            if (res == -1) {
                printf("Faild to get message from: %d, to: %d\n", i, 0);
                exit(EXIT_FAILURE);
            }
        }
        max_lamport_time(msg->s_header.s_local_time);
        

        // printf("%s", msg->s_payload);

        fprintf(events, "%s", msg->s_payload);
        fflush(events);

        mnemonic_pipe(pipes, i, 0, 0, msg->s_payload);
        fflush(pipes);
    }
    #ifdef STEPS
        printf("STEP 6: done read msg START\n");
    #endif


    /*

        тут был bank_robbery
    
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
        printf("STEP 7: read msg DONE\n");
    #endif
    header = newMessageHeader(MESSAGE_MAGIC, strlen(log_done_fmt), DONE, get_lamport_time());
    char newBuffer[MAX_PAYLOAD_LEN];
    msg = newMessage(header, newBuffer);

    // Получаю ото всех сообщения DONE
    for (int i = 1; i < manager->pidLen; i++) {

        int res = 1;
        while ((res = receive(manager, i, msg)) != 0) {
            if (res == -1) {
                printf("Faild to get message from: %d, to: %d\n", i, 0);
                return -1;
            }
        }
        max_lamport_time(msg->s_header.s_local_time);
    

        printf("%s", msg->s_payload);

        fprintf(events, "%s", msg->s_payload);
        fflush(events);

        mnemonic_pipe(pipes, i, 0, 0, msg->s_payload);
        fflush(pipes);

    }

    fprintf(events, log_received_all_done_fmt, get_lamport_time(), 0);
    fflush(events);
    


    /*
        Тут был баланс хистори
    */

    // Закрываю пайп
    #ifdef STEPS
        printf("STEP 8: close\n");
    #endif
    closeUsableFD(manager, 0, pipes);


    fclose(events);
    fclose(pipes);

    return 0;
}
