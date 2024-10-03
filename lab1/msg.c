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
