
#include "channel.h"

int findIdByPid(Manager *manager, int pid) {

    for (int i = 0; i < manager->pidLen; i++) {
        if (manager->pid[i].pid == pid)
            return i;
    }
    
    return -1;
}


Channel *newChannel(local_id from, local_id to) {
    Channel *chan = malloc(sizeof(Channel));

    chan->FromTo[0] = from;
    chan->FromTo[1] = to;

    return chan;
}

Process *newProcess(local_id id) {
    Process *lp = malloc(sizeof(Process));

    lp->id = id;
    lp->pid = -1;

    return lp;
}

Manager *newManager(int n) {
    Manager *manager = malloc(sizeof(Manager));

    manager->pidLen = n;
    manager->chanLen = n*(n-1);

    manager->pid = malloc(manager->pidLen * sizeof(Process));
    manager->chan = malloc(manager->chanLen  * sizeof(Channel));

    return manager;
}


void closeNotUsableFD(Manager *manager, local_id id, FILE *pipes) {

    for (int i = 0; i < manager->chanLen; i++) {
        // если from != id && to != id => close
        if (manager->chan[i].FromTo[0] != id && manager->chan[i].FromTo[1] != id) {
            //printf("Close PIPE BY PORC: %d \t from %d to %d\n", id, manager->chan[i].FromTo[0], manager->chan[i].FromTo[1]);
            close(manager->chan[i].PipesFd[0]);
            close(manager->chan[i].PipesFd[1]);

            fprintf(pipes, "Close not usable PIPE BY PROC: %d \t from %d to %d\n", id, manager->chan[i].FromTo[0], manager->chan[i].FromTo[1]);
            fflush(pipes);
        }

        // Тестовый вариант закрытия на запись если стоит на чтение
        else if (manager->chan[i].FromTo[1] == id) {
            close(manager->chan[i].PipesFd[1]);

        }

        // Тестовый вариант закрытия на чтение, если пишет
        else if (manager->chan[i].FromTo[0] == id) {
            close(manager->chan[i].PipesFd[0]);

        }

    }
}

void closeUsableFD(Manager *manager, local_id id, FILE *pipes) {
    for (int i = 0; i < manager->chanLen; i++) {
        if (manager->chan[i].FromTo[0] == id || manager->chan[i].FromTo[1] == id) {
            close(manager->chan[i].PipesFd[0]);
            close(manager->chan[i].PipesFd[1]);

            fprintf(pipes, "Close usable PIPE BY PORC: %d \t from %d to %d\n", id, manager->chan[i].FromTo[0], manager->chan[i].FromTo[1]);
            fflush(pipes);
        }

        
    }
}
