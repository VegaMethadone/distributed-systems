#include "interaction.h"

// #define DEBUG_INTER
// #define DEBUG_TRESIVER

int DONECOUNTER = 0;

int interaction(Manager *manager, FILE *events, FILE *pipes, int proc) {


    add_lamport_time();
    // Генерация сообщения STARTED
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, strlen(log_started_fmt), STARTED, get_lamport_time());
    char buffer [MAX_PAYLOAD_LEN];
    sprintf(buffer, log_started_fmt, get_lamport_time(), proc, getpid(), getppid(), 0);
    Message *msg = newMessage(header, buffer);
    


    // Посылка сообщения START
    send_multicast(manager, msg);
    //sleep(1);


    // Получаю ото всех сообщения START
    for (int i = 1; i < manager->pidLen; i++) {
        if (i == proc)
            continue;

        int res = 1;
        while ((res = receive(manager, i, msg)) != 0) {
            if (res == -1) {
                printf("Faild to get message from: %d, to: %d\n", i, proc);
                return -1;
            }
        }
        max_lamport_time(msg->s_header.s_local_time);
        
        // printf("%s", msg->s_payload);

        fprintf(events, "%s", msg->s_payload);
        fflush(events);

        mnemonic_pipe(pipes, i, proc, proc, msg->s_payload);
        fflush(pipes);
        
    }
    
    // Говорю о том, что процесс полуил все START сообщения
    fprintf(events, log_received_all_started_fmt,  get_lamport_time(), proc);
    fflush(events);

    sleep(2);


    /*


        ПОЛЕЗНАЯ РАБОТА


    */
    
    request_cs(manager);
    sleep(1);


    // Генерация сообщения DONE
    add_lamport_time();
    MessageHeader *doneHeader = newMessageHeader(MESSAGE_MAGIC, strlen(log_done_fmt), DONE, get_lamport_time());
    char doneBuffer[MAX_PAYLOAD_LEN];
    sprintf(doneBuffer, log_done_fmt, get_lamport_time(), proc, 0);
    Message *doneMsg = newMessage(doneHeader, doneBuffer);


    // Рассылка мултикастом о том, что процесс закончил работу
    send_multicast(manager, doneMsg);
    sleep(1);


    // Получаю сообщение о том, что  другие процессы DONE
    for (int i = 1; i < manager->pidLen; i++) {
        if (i == proc)
            continue;

        int res = 1;
        while ((res = receive(manager, i, doneMsg)) != 0) {
            if (res == -1) {
                printf("Faild to get message from: %d, to: %d\n", i, proc);
                return -1;
            }
        }
        max_lamport_time(doneMsg->s_header.s_local_time);

        printf("%s", msg->s_payload);

        fprintf(events, "%s", doneMsg->s_payload);
        fflush(events);

        mnemonic_pipe(pipes, i, proc, proc, doneMsg->s_payload);
        fflush(pipes);
    }
    

    // Отправляю сообщение о том, что процесс получил все DONE сообщения
    fprintf(events, log_received_all_done_fmt, get_lamport_time(), proc);
    fflush(events);


    return 0;
}



int request_cs(const void * self) {
    Manager *manager = (Manager *)self;

    pid_t pid = getpid();
    local_id id = findIdByPid(manager, pid);

    if (id == -1) {
        perror("invalid_id: request_cs");
        return - 1;
    }

    add_lamport_time();
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, 0, CS_REQUEST, get_lamport_time());
    Message *msg = newCS_Message(*header);

    push(manager->pid[id].Queue, newPair(id, get_lamport_time()));

    for (int i = 1; i < manager->pidLen; i++) {
        if (i == id) {
            continue;
        }
        int res = send(manager, i, msg);
        if (res != 0) {
            printf("Faild to send msg by request_cs by proc: %d\n", id);
            return -1;
        }
    }


    int actCounter = manager->pidLen-2;

    // Пока куча не будет поста - долбим
    while (manager->pid[id].Queue->len > 0) {
        int res = receive_any(manager, msg);

        if (actCounter == 0 && getMin(manager->pid[id].Queue).id == id) {

            // тест на принт
            int lenOfLogLoop = strlen(log_loop_operation_fmt);
            char lab4Buffer[lenOfLogLoop];
            int n = id*5;
            for (int i = 1; i <= n; i++) {
                sprintf(lab4Buffer, log_loop_operation_fmt, id, i, n);
                print(lab4Buffer);
            }
            printf("\n");
            // тест на принт

            release_cs(manager);
            pop(manager->pid[id].Queue);
        }
        
        // Если я получаю сообщение корретное
        if (res == 0 || res > 1) {
            max_lamport_time(msg->s_header.s_local_time);

            if (msg->s_header.s_type == CS_REQUEST) {
                push(manager->pid[id].Queue, newPair(res-10, msg->s_header.s_local_time));

                add_lamport_time();
                MessageHeader *repHeader = newMessageHeader(MESSAGE_MAGIC, 0, CS_REPLY, get_lamport_time());
                Message *repMsg = newCS_Message(*repHeader);

                send(manager, res-10, repMsg);

            }else if (msg->s_header.s_type == CS_REPLY) {
                actCounter--;

            }else if (msg->s_header.s_type == CS_RELEASE) {
                pop(manager->pid[id].Queue);

            }
            
        } else if (res == 1) {
            continue;

        }else {
            return -1;
        }

    } 

    return 0;
}


int release_cs(const void * self) {
    Manager *manager = (Manager *)self;

    pid_t pid = getpid();
    local_id id = findIdByPid(manager, pid);

    if (id == -1) {
        perror("invalid_id: release_cs");
        return - 1;
    }

    add_lamport_time();
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, 0, CS_RELEASE, get_lamport_time());
    Message *msg = newCS_Message(*header);

    for (int i = 1; i < manager->pidLen; i++) {
        if (i == id) {
            continue;
        }
        // printf("[ %d ] Send CS_RELEASE msg\n", id);
        int res = send(manager, i, msg);
        if (res != 0) {
            printf("Faild to send msg in release_cs by proc: %d\n", id);
            return -1;
        }
    }

    return 0;
}
