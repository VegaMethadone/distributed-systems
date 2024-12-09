#include "interaction.h"


// #define DEBUG_INTER
// #define DEBUG_TRESIVER

int DONECOUNTER = 0;
// тут будет массив из кол процессов, где индекс будет индефикатором процесса, а значение  0 или 1, где 1 - надо отослать реплай
int AWAITING[256];

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


    while ( true ) {
        // sleep(1);
        if (DONECOUNTER == manager->pidLen-2) {
            break;
        }

        int res = receive_any(manager, doneMsg);
        if (res > 1) {
            max_lamport_time(doneMsg->s_header.s_local_time);

            res = res - 10;
            
            if (doneMsg->s_header.s_type == CS_REQUEST) {
                continue;

            }else if (doneMsg->s_header.s_type == CS_RELEASE) {
                continue;

            }else if (doneMsg->s_header.s_type == CS_REPLY) {
                continue;
            }

        }else if (res == 1) {
            continue;

        }else if (res == 0) {
            max_lamport_time(doneMsg->s_header.s_local_time);
            if (doneMsg->s_header.s_type == DONE) {
                DONECOUNTER++;

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

/*

Я объединяю release + reply
Перед тем как войти в критическую секцию, я должен убеиться что у меня наименьший таймстемп

если пришел реквест, сраниваю время
    1. если время реквеста больше - отсылаю сразу же реплай
    2. если время реквеста меньше - я откладываю отправку сообщения типа реплай


*/

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


    timestamp_t currentTime = get_lamport_time();
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

    while (true) {
        int res = receive_any(manager, msg);


        if (actCounter + DONECOUNTER == manager->pidLen-2) {
            // printf("[ %d ] BREAK cause counter = %d\n", id, actCounter + DONECOUNTER);
            break;
        }

        
        if (res > 1) {

            // printf("[ %d ] Got msg from %d\n", id, res-10);
            max_lamport_time(msg->s_header.s_local_time);

            if (msg->s_header.s_type == CS_REQUEST) {

                /*
                    Сравнить время получения
                    если равно или больше -> я не отсылаю сейчас реплай, добавяю в массив 
                    если меньше, то я проиграл и отсылаю реплай
                */
                
                // timestamp_t currentTime = get_lamport_time();

                // если время входящего сообщения меньше, то я проиграл и отправляю реплай
                if (msg->s_header.s_local_time < currentTime) {
                    // printf("[ %d ] win to %d\t g:%d <- l:%d\n", id, res-10, msg->s_header.s_local_time, currentTime);

                    add_lamport_time();
                    MessageHeader *repHeader = newMessageHeader(MESSAGE_MAGIC, 0, CS_REPLY, get_lamport_time());
                    Message *repMsg = newCS_Message(*repHeader);

                    send(manager, res-10, repMsg);

                // если время входящего сообщения больше, то я выиграл и не отправляю реплай, а записываю в ожидающих
                }else if (msg->s_header.s_local_time > currentTime){
                    // printf("[ %d ] lose to %d\t g:%d => l:%d\n", id, res-10, msg->s_header.s_local_time, currentTime);
                    AWAITING[res-10] = 1;
                    // actCounter++;
                    // break;
                // иначе когда равны, приоритет тому, у кого id меньше
                }
                else {
                    if (res-10 < id) {
                        // printf("[ %d ] win to %d cause id\t g:%d == l:%d \n", id, res-10, msg->s_header.s_local_time, currentTime);
                        add_lamport_time();
                        MessageHeader *repHeader = newMessageHeader(MESSAGE_MAGIC, 0, CS_REPLY, get_lamport_time());
                        Message *repMsg = newCS_Message(*repHeader);

                        send(manager, res-10, repMsg);
                    }else {
                        // printf("[ %d ] lose to %d cause id\t g:%d == l:%d \n", id, res-10, msg->s_header.s_local_time, currentTime);
                        AWAITING[res-10] = 1;
                        // added cause if i got msg with bigger time it's the signal that it's ack and i don't have to wait for ack
                        // actCounter++;
                        // break;
                    }
                }

            }else if (msg->s_header.s_type == CS_REPLY) {
                actCounter++;
                // printf("[ %d ] Got REPLY from %d\t count = %d\n", id, res-10, actCounter+DONECOUNTER);

            }

        } else if (res == 1) {
            continue;

        }else if (res == 0 ) {
            if (msg->s_header.s_type == DONE) {
                // printf("[ %d ] Got DONE msg\n", id);
                DONECOUNTER++;

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

    // тут должен быть тип сообщения CS_REPLY
    add_lamport_time();
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, 0, CS_REPLY, get_lamport_time());
    Message *msg = newCS_Message(*header);

    // отправляю всем сообщение, кто заприашивал
    int res = 1;
    for (int i = 1; i < manager->pidLen; i++) {
        if (i == id) {
            continue;
        }
        if (AWAITING[i] == 1) {
            // printf("[ %d ] Send msg to %d from AWAITINS\n", id, i);
            res = send(manager, i, msg);
            if (res != 0) {
                printf("[ %d ] Faild to send msg in release_cs to proc: %d\n", id, i);
                return -1;
            }
        }
    }
    // подчищаю массив
    for (int i = 1; i < manager->pidLen; i++) {
        AWAITING[i] = 0;
    }

    return 0;
}
