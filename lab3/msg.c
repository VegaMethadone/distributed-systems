#include <stdio.h>
#include "msg.h"

//#define DEBUG_MSG
//#define DEBUG_HEADER

MessageHeader *newMessageHeader(uint16_t s_magic, uint16_t s_payload_len, int16_t s_type, timestamp_t s_local_time) {
    MessageHeader *header = malloc(sizeof(MessageHeader));
    

    header->s_magic = s_magic;
    header->s_payload_len = s_payload_len;
    header->s_type = s_type;
    header->s_local_time = s_local_time;

    #ifdef DEBUG_HEADER
        printf("MAGIC: got: %d -> set: %d\n", s_magic, header->s_magic);
        printf("PAYLOAD_LEN: got: %d -> set: %d\n", s_payload_len, header->s_payload_len);
        printf("TYPE: got: %d -> set: %d\n", s_type, header->s_type);
    #endif

    return header;
}


Message *newMessage(MessageHeader *header, const char *buffer) {
    Message *msg = malloc(sizeof(Message));

    msg->s_header = *header;
    strncpy(msg->s_payload, buffer, header->s_payload_len);

    #ifdef DEBUG_MSG
        printf("=================================\n");
        printf("MAGIC: %d\n", msg->s_header.s_magic);
        printf("PAYLOAD_LEN: %d\n", msg->s_header.s_payload_len);
        printf("D: %s", msg->s_payload);
        printf("B: %s", buffer);
        printf("=================================\n");
    #endif

    return msg;
}

Message *newTransferMessage(MessageHeader *header, TransferOrder *order) {
    Message *msg = malloc(sizeof(Message));

    msg->s_header = *header;
    memcpy(msg->s_payload, order, sizeof(TransferOrder));

    return msg;
}

Message *newStopMessage(timestamp_t time) {
    Message *msg = malloc(sizeof(Message));

    msg->s_header.s_local_time = time;
    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_type = STOP;
    
    msg->s_header.s_payload_len = 0;

    return msg;
}

Message *newAckMessage(timestamp_t time) {
    Message *msg = malloc(sizeof(Message));

    msg->s_header.s_local_time = time;
    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_type = ACK;

    msg->s_header.s_payload_len = 0;

    return msg;
}

Message *newBalanceHistoryMessage(BalanceHistory *history, timestamp_t time) {
    Message *msg = malloc(sizeof(Message));

    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_type = BALANCE_HISTORY;
    msg->s_header.s_local_time = time;
    msg->s_header.s_payload_len = sizeof(history->s_id) + sizeof(history->s_history_len) + sizeof(BalanceState) * history->s_history_len;

    memcpy(msg->s_payload, history, sizeof(history->s_id) + sizeof(history->s_history_len) + sizeof(BalanceState) * history->s_history_len);

    return msg;
}

Message *newEmptyBalanceHistoryMessage(void) {
    Message *msg = malloc(sizeof(Message));

    // Инициализация заголовка
    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_type = BALANCE_HISTORY;
    msg->s_header.s_local_time = 0;
    msg->s_header.s_payload_len = 0;

    char buffer[MAX_PAYLOAD_LEN];
    strncpy(msg->s_payload, buffer, MAX_PAYLOAD_LEN);

    return msg;
}

