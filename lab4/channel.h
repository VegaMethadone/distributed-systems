#ifndef DEFINE_CHANNEL
#define DEFINE_CHANNEL

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "pa2345.h"
#include "ipc.h"
#include "banking.h"
#include "queue.h"


typedef struct Process Process;
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
    queue *Queue; // added
};

struct Manager
{
    Process *pid;
    Channel *chan;
    int pidLen;
    int chanLen;
    int doneProcesses;
};

// lab 1
int findIdByPid(Manager *manager, int pid);

Manager *newManager(int n);
Process *newProcess(local_id id);
Channel *newChannel(local_id from, local_id to);

void closeUsableFD(Manager *manager, local_id id, FILE *pipes);
void closeNotUsableFD(Manager *manager, local_id id, FILE *pipes);

// lab3

void  add_lamport_time(void);
timestamp_t get_lamport_time(void);
void max_lamport_time(timestamp_t time);

#endif
