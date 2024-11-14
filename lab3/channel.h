#ifndef DEFINE_CHANNEL
#define DEFINE_CHANNEL

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include "stdbool.h"

#include "pa2345.h"
#include "ipc.h"
#include "banking.h"


typedef struct Process  Process;
typedef struct Channel Channel;
typedef struct Manager Manager;

struct Channel
{
    local_id FromTo[2];
    int PipesFd[2];
};

struct Process
{
    int pid;
    local_id id;
    BalanceState balance; // added
    BalanceHistory history; // added
};

struct Manager
{
    Process *pid;
    Channel *chan;
    int pidLen;
    int chanLen;
    AllHistory allHistroy; // added
};

int findIdByPid(Manager *manager, int pid);

Manager *newManager(int n);
Process *newProcess(local_id id);
Channel *newChannel(local_id from, local_id to);

void closeUsableFD(Manager *manager, local_id id, FILE *pipes);
void closeNotUsableFD(Manager *manager, local_id id, FILE *pipes);


// lab2
int findMaxT(AllHistory *history);
BalanceHistory newBalanceHistory(local_id id);
void printBalanceHistory(Manager *manager, local_id id);
void printBalanceHistoryNoManager(BalanceHistory *history);
void restoreBalanceInTheEnd(AllHistory *history, timestamp_t time);
BalanceState newBalanceState(balance_t s_balance, timestamp_t s_time);
void restoreBalanceState(Manager *manager, local_id id, timestamp_t newTime);
void addBalanceState(Manager *manager, local_id id, BalanceState balanceState);

// lab3
void  add_lamport_time(void);
timestamp_t get_lamport_time(void);
void max_lamport_time(timestamp_t time);
void restorePendingBalance(BalanceHistory *history, timestamp_t sendTime, timestamp_t currentTime, balance_t balance);

#endif
