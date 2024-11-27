#include "interaction.h"


// #define DEBUG_INTER
// #define DEBUG_TRESIVER

// lab4(copy4)
int DONECOUNTER = 0;

int interaction(Manager *manager, FILE *events, FILE *pipes, int proc, bool mutexed) {


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
    // manager->doneProcesses = manager->pidLen-2;

    // char deugBuffer[128];
    // sprintf(buffer, "[ %d ] Start loop\n", proc);
    // print(buffer);

    char lab4Buffer[128];
    for (int i = 1; i <= proc*5; i++) {
        if (mutexed) {
            request_cs(manager);
        }
        memset(lab4Buffer, 0, sizeof(lab4Buffer));
        sprintf(lab4Buffer, log_loop_operation_fmt, proc, i, proc*5);
        print(lab4Buffer);

        if (mutexed) {
            release_cs(manager);
        }
    }
    // printf("[ %d ] Loop done\n", proc);
    sleep(1);
    // sprintf(deugBuffer, "[ %d ] Done loop\n", proc);
    // print(deugBuffer);


    // Генерация сообщения DONE
    add_lamport_time();
    MessageHeader *doneHeader = newMessageHeader(MESSAGE_MAGIC, strlen(log_done_fmt), DONE, get_lamport_time());
    char doneBuffer[MAX_PAYLOAD_LEN];
    sprintf(doneBuffer, log_done_fmt, get_lamport_time(), proc, 0);
    Message *doneMsg = newMessage(doneHeader, doneBuffer);


    // Рассылка мултикастом о том, что процесс закончил работу
    // printf("============ %d ============== send DONE MSG\n", proc);
    send_multicast(manager, doneMsg);
    sleep(1);


    // 
    // printf("[ %d ] DONECOUNTER IS: %d, TARGET: %d\n", proc, DONECOUNTER, manager->pidLen-2);
    while ( true ) {
        // sleep(1);
        // printf("[ %d ] is waiting, DONE IS: %d\n", proc, DONECOUNTER);
        // printf("[ %d ] DoneCounter is %d\n", proc, DONECOUNTER);
        if (DONECOUNTER == manager->pidLen-2) {
            break;
        }

        int res = receive_any(manager, doneMsg);
        if (res > 1) {
            max_lamport_time(doneMsg->s_header.s_local_time);

            res = res - 10;
            
            if (doneMsg->s_header.s_type == CS_REQUEST) {
                // printf("[ %d ] AFTER Got CS_REQUEST msg\n", proc);

                // add_lamport_time();
                // MessageHeader *repHeader = newMessageHeader(MESSAGE_MAGIC, 0, CS_REPLY, get_lamport_time());
                // Message *repMsg = newCS_Message(*repHeader);

                // printf("[ %d ] AFTER Send REPLY to %d\n", proc, res);
                // send(manager, res, repMsg);
                continue;

            }else if (doneMsg->s_header.s_type == CS_RELEASE) {
                // printf("[ %d ] AFTER Got CS_RELEASE msg\n", proc);
                continue;

            }else if (doneMsg->s_header.s_type == CS_REPLY) {
                // printf("[ %d ] AFTER Got CS_REPLY msg\n", proc);
                continue;
            }

        }else if (res == 1) {
            continue;

        }else if (res == 0) {
            max_lamport_time(doneMsg->s_header.s_local_time);
            if (doneMsg->s_header.s_type == DONE) {
                // printf("[ %d ] AFTER Got donne msg\n", proc);
                DONECOUNTER++;

                // printf("[ %d ] %s", proc, doneMsg->s_payload);
                fprintf(events, "%s", doneMsg->s_payload);
                fflush(events);


            }else {
                printf("[ %d ] Got unexpected msg type: %d\n", proc, doneMsg->s_header.s_type);
                return -1;
            }
        }

    }
    sleep(2);
    // printf("[ %d ] Send evetns\n", proc);
    // Отправляю сообщение о том, что процесс получил все DONE сообщения
    fprintf(events, log_received_all_done_fmt, get_lamport_time(), proc);
    fflush(events);

    printf("[ %d ] Exiting\n", proc);
    return 0;
}



//  что я буду делать, если я получил сообщение типа Done. Останется ли у меня в буфере это ?
// если не останется, то куда будет положенно сообщение, если нет места под него
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
        // printf("[ %d ] Send message to proc: %d\n", id,  i);
        int res = send(manager, i, msg);
        if (res != 0) {
            printf("Faild to send msg by request_cs by proc: %d\n", id);
            return -1;
        }
    }


    int actCounter = 0;

    // Пока куча не будет поста - долбим
    while (true) {
        // printf("[ %d ] trying get msg\n", id);
        int res = receive_any(manager, msg);

        if ((actCounter + DONECOUNTER == manager->pidLen-2) &&  manager->pid[id].Queue->len > 0 && getMin(manager->pid[id].Queue).id == id) {
            // printf("[ %d ] actCounter == %d && queue[0].id == %d\n", id, manager->pidLen-2, manager->pid[id].Queue->arr[0].id);
            // printf("[ %d ] -> HEAP:\t", id);
            pop(manager->pid[id].Queue);
            // printf("[ %d ] -> HEAP:\t", id);
            // printQueue(manager->pid[id].Queue);
            break;
        }
        
        // Если я получаю сообщение корретное
        if (res > 1) {
            // printf("[ %d ] Got msg from proc: %d\n", id, res-10);
            // printf("[ %d ] Got type_msg: %d\n", id, msg->s_header.s_type);
            max_lamport_time(msg->s_header.s_local_time);


            if (msg->s_header.s_type == CS_REQUEST) {
                // printf("[ %d ] Got CS_REQUEST\n", id);
                push(manager->pid[id].Queue, newPair(res-10, msg->s_header.s_local_time));
                // printf("[ %d ] -> HEAP:\t", id);
                // printQueue(manager->pid[id].Queue);


                add_lamport_time();
                MessageHeader *repHeader = newMessageHeader(MESSAGE_MAGIC, 0, CS_REPLY, get_lamport_time());
                Message *repMsg = newCS_Message(*repHeader);

                // printf("[ %d ] Send msg to %d\n", id, res-10);
                send(manager, res-10, repMsg);

            }else if (msg->s_header.s_type == CS_REPLY) {
                actCounter++;
                // printf("[ %d ] Got CS_REPLY, actCounter: %d\n", id, actCounter);

            }else if (msg->s_header.s_type == CS_RELEASE) {
                // printf("[ %d ] Got CS_RELEASE\n", id);
                pop(manager->pid[id].Queue);
                // printf("[ %d ] -> HEAP:\t", id);
                // printQueue(manager->pid[id].Queue);
            }

        } else if (res == 1) {
            continue;

        }else if (res == 0 ) {
            if (msg->s_header.s_type == DONE) {
                DONECOUNTER++;
                // printf("[ %d ] got msg DONE: %d, actCounter: %d\n", id, DONECOUNTER, actCounter);

            }else {
                printf("[ %d ] Got unexpected msg with type %d\n", id, msg->s_header.s_type);
                return -1;
            }
        } else {
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
