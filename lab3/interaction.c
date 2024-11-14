#include "interaction.h"

#define DEBUG_INTER

#define DEBUG_TRESIVER

int interaction(Manager *manager, FILE *events, FILE *pipes, int proc) {

    // Добавление начального баланса в историю
    addBalanceState(manager, proc, manager->pid[proc].balance);
    manager->pid[proc].history.s_id = proc;


    add_lamport_time();
    // Генерация сообщения STARTED
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, strlen(log_started_fmt), STARTED, get_lamport_time());
    char buffer [MAX_PAYLOAD_LEN];
    sprintf(buffer, log_started_fmt, get_lamport_time(), proc, getpid(), getppid(), manager->pid[proc].balance.s_balance);
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

        ожидание сообщений двух типов TRANSFER & STOP

    */
    header = newMessageHeader(MESSAGE_MAGIC, sizeof(TransferOrder), TRANSFER, get_lamport_time());
    TransferOrder order = { 0, 0 ,0};
    Message *tMsg = newTransferMessage(header, &order);

    while (true) {
        int res = receive_any(manager, tMsg);
        if(res != 0) {
            if (res == -1) {
                printf("Faild to read transfer msg by proc: %d\n", proc);
                return -1;
            }
            // Так как данные еще не пришли, ждем дальше любые данные
            continue;
        }
        max_lamport_time(tMsg->s_header.s_local_time);

        // Если пришло сообщение с типом STOP - выходим из цикла
        if (tMsg->s_header.s_type == STOP) {
            break;
        }

        // Если пришло сообщение с типом TRANSFER - определяем вариант события
        if (tMsg->s_header.s_type == TRANSFER) {
            // printf("Got transfer msg by proc: %d\n", proc);
            TransferOrder *tBody = (TransferOrder*)tMsg->s_payload;
            
            // BalanceState prevState = manager->pid[proc].balance;

            // Восстанавливаю баланс, если t = 1 и пришло t = 4. Расставить t = 2 & t = 3
            // printf("RESTORING BALANCE BY PROC: %d\n", proc);
            restoreBalanceState(manager, proc, get_lamport_time());

            // Если proc == src -> вычесть из баланса и передать сообщение дальше 
            // Если proc == dst -> добавить на свой счет и отправить родителю ACK
            if (tBody->s_dst == proc) {


                // Добавляю на счет, если эо dst
                manager->pid[proc].balance.s_balance += tBody->s_amount;
                manager->pid[proc].balance.s_time = get_lamport_time();

                restorePendingBalance(&manager->pid[proc].history, tMsg->s_header.s_local_time, get_lamport_time(), tBody->s_amount);
                
                fprintf(events, log_transfer_in_fmt, get_lamport_time(), proc,  tBody->s_amount, tBody->s_src);
                fflush(events);



                // printBalanceHistory(manager, proc);
                addBalanceState(manager, proc, manager->pid[proc].balance);
                // printBalanceHistory(manager, proc);

                // добавил
                add_lamport_time();

                Message *ackMsg = newAckMessage(get_lamport_time());
                send(manager, 0, ackMsg);


            } else {
                // Беру новое время процесса т.к. новое действие



                manager->pid[proc].balance.s_balance -= tBody->s_amount;
                manager->pid[proc].balance.s_time = get_lamport_time();

                fprintf(events, log_transfer_out_fmt, get_lamport_time(), proc, tBody->s_amount, tBody->s_dst);
                fflush(events);


                // printBalanceHistory(manager, proc);
                addBalanceState(manager, proc, manager->pid[proc].balance);
                // printBalanceHistory(manager, proc);


                add_lamport_time();
                tMsg->s_header.s_local_time = get_lamport_time();
                send(manager, tBody->s_dst, tMsg);
            }
             

        }

    }




























   


    // Генерация сообщения DONE
    add_lamport_time();
    MessageHeader *doneHeader = newMessageHeader(MESSAGE_MAGIC, strlen(log_done_fmt), DONE, get_lamport_time());
    char doneBuffer[MAX_PAYLOAD_LEN];
    sprintf(doneBuffer, log_done_fmt, get_lamport_time(), proc, manager->pid[proc].balance.s_balance);
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


















    // printf("Prepare Balance MSG to tansfer by proc: %d\n", proc);
    // Отправляю папке сообщение типа BALANCE_HISTORY с данными о перевеоде
    add_lamport_time();
    Message *bMsg = newBalanceHistoryMessage(&manager->pid[proc].history, get_lamport_time());
    // printf("Test balance msg by proc: %d\n", proc);
    // BalanceHistory *checkB = (BalanceHistory*)bMsg->s_payload;
    // printBalanceHistoryNoManager(checkB);

    // printf("Send Balance MSG to parent by proc: %d\n", proc);
    int res = send(manager, 0, bMsg);
    if (res != 0) {
        printf("Faild to send Balance MSG by proc: %d\n", proc);
        return -1;
    }
    // else {
    //     printf("Seccessfuly send balance msg by proc: %d\n", proc);
    // }






    return 0;
}
