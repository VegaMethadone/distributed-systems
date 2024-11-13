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


BalanceHistory newBalanceHistory(local_id id) {
    BalanceHistory *bh = malloc(sizeof(BalanceHistory));
    bh->s_id = id;
    bh->s_history_len = 0;

    return *bh;
}

Process *newProcess(local_id id) {
    Process *lp = malloc(sizeof(Process));

    BalanceState balance = {.s_balance = -1, .s_balance_pending_in = 0, .s_time = -1};

    lp->id = id;
    lp->pid = -1;
    lp->balance = balance;

    // Создаю историю для процесса
    lp->history = newBalanceHistory(id);

    return lp;
}

Manager *newManager(int n) {
    Manager *manager = malloc(sizeof(Manager));

    manager->pidLen = n;
    manager->chanLen = n*(n-1);

    manager->pid = malloc(manager->pidLen * sizeof(Process));
    manager->chan = malloc(manager->chanLen  * sizeof(Channel));

    AllHistory *allHis = malloc(sizeof(AllHistory));
    manager->allHistroy = *allHis;

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

BalanceState newBalanceState(balance_t s_balance, timestamp_t s_time) {
    BalanceState currentBalance = {
        .s_balance = s_balance,
        .s_time = s_time,
        .s_balance_pending_in = 0,
    };

    return currentBalance;
}

void addBalanceState(Manager *manager, local_id id, BalanceState balanceState) {
    uint8_t *len = &manager->pid[id].history.s_history_len;
    manager->pid[id].history.s_history[*len] = balanceState;
    (*len)++;
    // if (balanceState.s_time == 0) {
    //     printf("Added start balance state to proc: %d. Size:%d\n", id, (*len));
    // }
} 

void restoreBalanceState(Manager *manager, local_id id, timestamp_t newTime) {
    uint8_t *len = &manager->pid[id].history.s_history_len;
    if((*len) == 0) {
        printf("History in proc: %d is empty!\n", id);
        return;
    }

    while (manager->pid[id].balance.s_time+1 < newTime) {
        manager->pid[id].balance.s_time++;
        manager->pid[id].history.s_history[(*len)] = manager->pid[id].balance;

        (*len)++;
    }
}

void restoreBalanceInTheEnd(AllHistory *history, timestamp_t time) {
    for (int i = 0; i < history->s_history_len; i++) {
        if (history->s_history[i].s_history[history->s_history[i].s_history_len-1].s_time < time) {
            BalanceState prev = history->s_history[i].s_history[history->s_history[i].s_history_len-1];

            while (prev.s_time < time) {
                prev.s_time++;
                history->s_history[i].s_history[history->s_history[i].s_history_len] = prev;
                history->s_history[i].s_history_len++;
            }
        }
    }
}


int findMaxT(AllHistory *history) {
    int max = 0;
    for (int i = 0; i < history->s_history_len; i++) {
        for (int j = 0; j < history->s_history[i].s_history_len; j++) {
            if (history->s_history[i].s_history[j].s_time > max) {
                max = history->s_history[i].s_history[j].s_time;
            }
        }
    }

    return max;
}

void printBalanceHistory(Manager *manager, local_id id) {
    printf("==============%d=============\n", id);
    printf("Amount of states: %d\n", manager->pid[id].history.s_history_len);
    for (int i = 0; i < manager->pid[id].history.s_history_len; i++) {
        printf("index: %d, balance: %d, time: %d\n", i,
        manager->pid[id].history.s_history[i].s_balance,
        manager->pid[id].history.s_history[i].s_time
        );
    }
    printf("============================\n");
}

void printBalanceHistoryNoManager(BalanceHistory *history) {
    printf("==============%d=============\n", history->s_id);
    printf("Amount of states: %d\n", history->s_history_len);
    for (int i = 0; i < history->s_history_len; i++) {
        printf("index: %d, balance: %d, time: %d\n", i,
        history->s_history[i].s_balance,
        history->s_history[i].s_time
        );
    }
    printf("============================\n");
}
