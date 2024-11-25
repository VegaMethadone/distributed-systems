#ifndef INTER_DEFINITION
#define INTER_DEFINITION


#define mnemonic_pipe(logfile, from, to, proc, mes) \
    fprintf(logfile, "Pipe %d -> %d \t Process %d got msg: %s", from, to, proc, mes);

#include "channel.h"

int interaction(Manager *manager, FILE *events, FILE *pipes, int proc);

#endif
