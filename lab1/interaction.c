#include "interaction.h"

#define DEBUG_INTER


int interaction(Manager *manager, FILE *events, FILE *pipes, int proc) {

    // Генерация сообщения STARTED
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, 48, STARTED, time(NULL));
    char buffer [MAX_PAYLOAD_LEN];
    sprintf(buffer, log_started_fmt, proc, getpid(), getppid());
    Message *msg = newMessage(header, buffer);
    
    //printf("STARTED -> LEN: %d \t MSG: %s", msg->s_header.s_payload_len, msg->s_payload);


    // Посылка сообщения START
    send_multicast(manager, msg);
    sleep(1);


    // Получаю ото всех сообщения START
    for (int i = 1; i < manager->pidLen; i++) {
        if (i == proc)
            continue;

        int res = receive(manager, i, msg);
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
    
    // Говорю о том, что процесс полуил все START сообщения
    fprintf(events, log_received_all_started_fmt, proc);
    fflush(events);




    /*


        ПОЛЕЗНАЯ РАБОТА


    */




    // Генерация сообщения DONE
    MessageHeader *doneHeader = newMessageHeader(MESSAGE_MAGIC, 28, DONE, time(NULL));
    char doneBuffer[MAX_PAYLOAD_LEN];
    sprintf(doneBuffer, log_done_fmt, proc);
    Message *doneMsg = newMessage(doneHeader, doneBuffer);

    //printf("DONE -> LEN: %d \t MSG: %s", doneMsg->s_header.s_payload_len, doneMsg->s_payload);

    // Рассылка мултикастом о том, что процесс закончил работу
    send_multicast(manager, doneMsg);
    sleep(1);


    // Получаю сообщение о том, что  другие процессы DONE
    for (int i = 1; i < manager->pidLen; i++) {
        if (i == proc)
            continue;

        int res = receive(manager, i, doneMsg);
        if (res == -1) {
            printf("Faild to get message from: %d, to: %d\n", i, proc);
            return -1;
        }

        printf("%s", msg->s_payload);

        fprintf(events, "%s", doneMsg->s_payload);
        fflush(events);

        mnemonic_pipe(pipes, i, proc, proc, doneMsg->s_payload);
        fflush(pipes);
    }
    

    // Отправляю сообщение о том, что процесс получил все DONE сообщения
    fprintf(events, log_received_all_done_fmt, proc);
    fflush(events);


    return 0;
}
