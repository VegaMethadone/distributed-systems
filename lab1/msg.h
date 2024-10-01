#ifndef MSG_DEFINITION
#define MSG_DEFINITION

#include <string.h>
#include <stdlib.h>

#include "ipc.h"





MessageHeader *newMessageHeader(uint16_t s_magic, uint16_t s_payload_len, int16_t s_type, timestamp_t s_local_time);
Message *newMessage(MessageHeader *header, const char *buffer);


#endif
