#include "channel.h"


int send_multicast(void * self, const Message * msg) {
    Manager *manager = (Manager *)self;

    pid_t pid = getpid();
    local_id id = findIdByPid(manager, pid);

    if (id == -1) {
        perror("invalid_id: send_multicast");
        return - 1;
    }

    for (int i = 0; i < manager->chanLen; i++) {
        if (manager->chan[i].FromTo[0] == id) {
            int res = write(manager->chan[i].PipesFd[1], msg, MAX_MESSAGE_LEN);
            if (res == -1)
                return -1;
        }
    }
    
    return 0;
}

int receive(void * self, local_id from, Message * msg) {
    Manager *manager = (Manager *)self;

    pid_t pid = getpid();
    local_id id = findIdByPid(manager, pid);

    if (id == -1) {
        perror("invalid_id: receive");
        return - 1;
    }

    for (int i = 0; i < manager->chanLen; i++) {
        if (manager->chan[i].FromTo[0] == from && manager->chan[i].FromTo[1] == id) {
            
            int res = read(manager->chan[i].PipesFd[0], msg, MAX_MESSAGE_LEN);

            if (res == -1) {
                perror("recieve");
                return -1;
            }

            return 0;
        }
    }

    return -1;
}

int receive_any(void * self, Message * msg) {
    Manager *manager = (Manager *)self;

    pid_t pid = getpid();
    local_id id = findIdByPid(manager, pid);

    if (id == -1) {
        perror("invalid_id: receive_any");
        return - 1;
    }
    
    /*
    bool isHost = false;
    if (id == 0) {
        isHost = true;
    }
    */

    for (int i = 0; i < manager->chanLen; i++) {
        if (manager->chan[i].FromTo[1] == id) {
            // Игнорирую случай сообщения от родителя так как он не посылает сообщения
            if (manager->chan[i].FromTo[0] == 0) {
                continue;
            }


            printf("Trying read data from: %d to: %d\n", manager->chan[i].FromTo[0], manager->chan[i].FromTo[1]);
            int fd = manager->chan[i].PipesFd[0];

            // Устанавливаю флаги для неблокирующей операции
            int flags = fcntl(fd, F_GETFL);
            if (flags == -1) {
                perror("fcntl get flags");
                return -1;
            }
            if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
                perror("fcntl set flags");
                return -1;
            }

            // Читаю из канала, куда мне писали
            ssize_t bytes = read(fd, msg, MAX_MESSAGE_LEN);
            if (bytes < 0) {
                perror("read");
                return -1;

            }else {
                return 0;

            }
        }
    }
    return -1;
}
