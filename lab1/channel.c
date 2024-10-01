
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
