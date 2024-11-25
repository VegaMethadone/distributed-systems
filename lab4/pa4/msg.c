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


    return header;
}


Message *newMessage(MessageHeader *header, const char *buffer) {
    Message *msg = malloc(sizeof(Message));

    msg->s_header = *header;
    strncpy(msg->s_payload, buffer, header->s_payload_len);

    return msg;
}


Message *newCS_Message(MessageHeader header) {
    Message *msg = malloc(sizeof(Message));

    msg->s_header = header;

    return msg;
}
