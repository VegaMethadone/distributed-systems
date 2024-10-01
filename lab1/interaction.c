#include "interaction.h"


int interaction(Manager *manager, FILE *events, FILE *pipes, int proc) {

    // Генерация сообщения STARTED
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, MAX_PAYLOAD_LEN, STARTED, time(NULL));
    char buffer[MAX_PAYLOAD_LEN];
    sprintf(buffer, log_started_fmt, proc, getpid(), getppid());
    Message *msg = newMessage(header, buffer);        


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
    header = newMessageHeader(MESSAGE_MAGIC, MAX_PAYLOAD_LEN, DONE, time(NULL));
    sprintf(buffer, log_done_fmt, proc);
    msg = newMessage(header, buffer);

    // Рассылка мултикастом о том, что процесс закончил работу
    send_multicast(manager, msg);
    sleep(1);


    // Получаю сообщение о том, что  другие процессы DONE
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
    

    // Отправляю сообщение о том, что процесс получил все DONE сообщения
    fprintf(events, log_received_all_done_fmt, proc);
    fflush(events);


    return 0;
}
