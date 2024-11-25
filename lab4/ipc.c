#include "string.h"
#include "channel.h"


int send(void * self, local_id dst, const Message * msg) {
    Manager *manager = (Manager *)self;

    pid_t pid = getpid();
    local_id id = findIdByPid(manager, pid);

    if (id == -1) {
        perror("invalid_id: send_multicast");
        return - 1;
    }

    for (int i = 0; i < manager->chanLen; i++) {
        if (manager->chan[i].FromTo[0] == id && manager->chan[i].FromTo[1] == dst) {
            int res = write(manager->chan[i].PipesFd[1], msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
            if (res == -1) {
                printf("Proc: %d filed to send msg \t from: %d, to: %d\n",  id, manager->chan[i].FromTo[0], manager->chan[i].FromTo[1]);
                return -1;
            }
        }
    }

    return 0;
}


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
            int res = write(manager->chan[i].PipesFd[1], msg, sizeof(MessageHeader)+ msg->s_header.s_payload_len);
            if (res == -1) {
                printf("Proc: %d filed to send msg \t from: %d, to: %d\n",  id, manager->chan[i].FromTo[0], manager->chan[i].FromTo[1]);
                return -1;
            }

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
            
            // Читаю сам хедер для определения, какой длины сообщение читать из дескриптера
            int res = read(manager->chan[i].PipesFd[0], &msg->s_header, sizeof(MessageHeader));

            if (res == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    return 1;
                }
            }
            

            // Проверяю на ошибку
            if (res == -1) {
                perror("Faild to read: receive");
                printf("Proc: %d \t from: %d -> to: %d \t read: %d, write: %d\n", id, manager->chan[i].FromTo[0], manager->chan[i].FromTo[1], manager->chan[i].PipesFd[0], manager->chan[i].PipesFd[1]);
                return -1;
            }


            if (msg->s_header.s_type == ACK) {
                return 0;
            }

            // Создаю буффер на стеке и читаю в него данные из дескриптера, после копирую n байтов в msg.s_payload
            //char buffer[MAX_PAYLOAD_LEN];
            res = read(manager->chan[i].PipesFd[0], &msg->s_payload, msg->s_header.s_payload_len);
            //strncpy(msg->s_payload, buffer, msg->s_header.s_payload_len);

            // Проверяю на ошибку
            if (res == -1) {
                perror("Faild to read: receive");
                printf("Proc: %d \t from: %d -> to: %d \t read: %d, write: %d\n", id, manager->chan[i].FromTo[0], manager->chan[i].FromTo[1], manager->chan[i].PipesFd[0], manager->chan[i].PipesFd[1]);
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

    for (int i = 0; i < manager->chanLen; i++) {
        if (manager->chan[i].FromTo[1] == id) {



            ssize_t bytes_read = read(manager->chan[i].PipesFd[0], &msg->s_header, sizeof(MessageHeader));
            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                }else {
                    perror("read_any: read header");
                    return -1;
                }
            }

            // Обрабатываю случаю STOP
            if (msg->s_header.s_type == STOP) {
                //printf("Proc %d got STOP header. Return... 0\n", id);
                return 0;
            }
            // Найти по пайпу
            if (msg->s_header.s_type == CS_REQUEST || msg->s_header.s_type == CS_REPLY || msg->s_header.s_type == CS_RELEASE) {
                int fromid = manager->chan[i].FromTo[0];
                return fromid+10;
            }


            bytes_read = read(manager->chan[i].PipesFd[0], &msg->s_payload, msg->s_header.s_payload_len);
            if (bytes_read == -1) {
                // сомнитально, но окей
                    perror("read_any: read body");
                    return -1;
            }

            if (bytes_read > 0) {
                //printf("GOT BODY BY PROC: %d WITH TYPE: %d\n", id, msg->s_header.s_type);
                return 0;
            }


        }
    }

    return 1;
}
