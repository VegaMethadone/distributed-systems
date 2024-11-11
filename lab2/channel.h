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

Channel *newChannel(local_id from, local_id to);
Process *newProcess(local_id id);
Manager *newManager(int n);

void closeNotUsableFD(Manager *manager, local_id id, FILE *pipes);
void closeUsableFD(Manager *manager, local_id id, FILE *pipes);


// new
BalanceHistory newBalanceHistory(local_id id);
BalanceState newBalanceState(balance_t s_balance, timestamp_t s_time);
void printBalanceHistory(Manager *manager, local_id id);
void restoreBalanceState(Manager *manager, local_id id, timestamp_t newTime);
void addBalanceState(Manager *manager, local_id id, BalanceState balanceState);
void printBalanceHistoryNoManager(BalanceHistory *history);
int findMaxT(AllHistory *history);
void restoreBalanceInTheEnd(AllHistory *history, timestamp_t time);

#endif
