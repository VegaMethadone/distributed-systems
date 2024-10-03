#ifndef INTER_DEFINITION
#define INTER_DEFINITION


#define mnemonic_pipe(logfile, from, to, proc, mes) \
    fprintf(logfile, "Pipe %d -> %d \t Process %d got msg: %s", from, to, proc, mes);

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>


#include "msg.h"
#include "pa1.h"
#include "common.h"
#include "channel.h"

int interaction(Manager *manager, FILE *events, FILE *pipes, int proc);

#endif
