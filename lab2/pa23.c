#include "banking.h"
#include "interaction.h"


#define STEPS



void transfer(void * parent_data, local_id src, local_id dst,
              balance_t amount)
{


    TransferOrder order = { src, dst, amount };
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, sizeof(TransferOrder), TRANSFER, get_physical_time());
    Message *transferMsg = newTransferMessage(header, &order);

    int res = send(parent_data, src, transferMsg);
    if (res != 0) {
        printf("Faild to send msg from %d to %d\n", src, dst);
        exit(EXIT_FAILURE);
    }

    /*
        ПОлучаю тут ACK сообщение от процесса, куда отправлял TRANSER MESSAGE
    */

    Message *ackMsg = newAckMessage(0);
    res = receive(parent_data, dst, ackMsg);
    if (res != 0) {
        printf("Faild to get ACK msg from %d to %d\n", dst, 0);
        exit(EXIT_FAILURE);
    }

}



void printMessageType(void) {
    printf("STARTED - %d\n", STARTED);
    printf("DONE - %d\n", DONE);
    printf("ACK - %d\n", ACK);
    printf("TRANSFER - %d\n", TRANSFER);
    printf("BALANCE_HISTORY - %d\n", BALANCE_HISTORY);
}



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

            // Получаю баланс 
            int startBalance = strtol(args[2+i], NULL, 10);
            manager->pid[i].balance.s_balance = startBalance;
            manager->pid[i].balance.s_time = get_physical_time();


            // Выполнение работы дочерним процессом
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
    // Также закрываю пайпы на запись в родительском процессе т.к. он только мониторит
    manager->pid[0].pid = getpid();
    closeNotUsableFD(manager, 0, pipes);



    #ifdef STEPS
        printf("STEP 4: message\n");
    #endif
    // тут вместо 48 перед STARTED поставил strlen(log_started_fmt) + добавил get_physical_time
    MessageHeader *header = newMessageHeader(MESSAGE_MAGIC, strlen(log_started_fmt), STARTED, time(NULL));
    char buffer[MAX_PAYLOAD_LEN];
    sprintf(buffer, log_started_fmt, 0, 0, getpid(), getppid(), 0);
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
            exit(EXIT_FAILURE);
        }

        printf("%s", msg->s_payload);

        fprintf(events, "%s", msg->s_payload);
        fflush(events);

        mnemonic_pipe(pipes, i, proc, proc, msg->s_payload);
        fflush(pipes);
    }


    /*
        Тут я должен вызвать функцию bank_robbery(), которая выполняет ряд перевеодов денег 
        между произвольными проессам

    */

    #ifdef STEPS
        printf("STEP 6: bank robbery\n");
    #endif
    sleep(2);
    bank_robbery(manager, n-1);

    Message *stopMsg = newStopMessage(time(NULL));
    send_multicast(manager, stopMsg);


    /*
        После я отправляю всем сообщение STOP и ожидаю сообщения DONE ото всех дочерних процессов

        После процесс должен получить ото всех  дочерних элементов структуру BalanceHistroy
        Которая после передается как аргумент в функцию print_history()
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
    // strlen вместо 28 перед done
    header = newMessageHeader(MESSAGE_MAGIC, strlen(log_done_fmt), DONE, time(NULL));
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

    fprintf(events, log_received_all_done_fmt, get_physical_time(), 0);
    fflush(events);
    


    /*
        Получаю ото всех сообщения balance_history
        print_history()
    */ 
    manager->allHistroy.s_history_len = n-1;
    printf("MAIN IS GETTING BALANCE_HYSTORY\n");
    Message *bMsg = newEmptyBalanceHistoryMessage();
    for (int i = 1; i < manager->pidLen; i++) {
        
        int res = receive(manager, i, bMsg);
        if (res == -1) {
            printf("Faild to get BALANCE_HISTORY from: %d to: 0\n", i);
            return -1;
        }
        if (bMsg->s_header.s_type != BALANCE_HISTORY) {
            printf("Expected BALANCE_HISTORY, got: %d\n", bMsg->s_header.s_type);
            printMessageType();
            return -1;
        }
        
        BalanceHistory *currentHistory = (BalanceHistory*)bMsg->s_payload;
        //printBalanceHistoryNoManager(currentHistory);

        manager->allHistroy.s_history[currentHistory->s_id-1] = *currentHistory;
    }

    int max = findMaxT(&manager->allHistroy);
    printf("FOUND MAX T: %d\n", max);
    restoreBalanceInTheEnd(&manager->allHistroy, max);

    for (int i = 1; i <= manager->allHistroy.s_history_len; i++) {
        printBalanceHistoryNoManager(&manager->allHistroy.s_history[i]);
    }  

    printf("\n\n\n");
    print_history(&manager->allHistroy);

    // Закрываю пайп
    #ifdef STEPS
        printf("STEP 8: close\n");
    #endif
    closeUsableFD(manager, 0, pipes);


    fclose(events);
    fclose(pipes);

    return 0;
}
