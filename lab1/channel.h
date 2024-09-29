#ifndef DEFINE_CHANNEL
#define DEFINE_CHANNEL

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "stdbool.h"

#include "pa1.h"
#include "ipc.h"


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
};

struct Manager
{
    Process *pid;
    Channel *chan;
    int pidLen;
    int chanLen;
};

int findIdByPid(Manager *manager, int pid);

Channel *newChannel(local_id from, local_id to);
Process *newProcess(local_id id);
Manager *newManager(int n);


#endif
