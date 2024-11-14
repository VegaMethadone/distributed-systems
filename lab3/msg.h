#ifndef MSG_DEFINITION
#define MSG_DEFINITION

#include <string.h>
#include <stdlib.h>

#include "ipc.h"
#include "banking.h"





MessageHeader *newMessageHeader(uint16_t s_magic, uint16_t s_payload_len, int16_t s_type, timestamp_t s_local_time);
Message *newMessage(MessageHeader *header, const char *buffer);

// NEW
Message *newTransferMessage(MessageHeader *header, TransferOrder *order);
Message *newStopMessage(timestamp_t time);
Message *newAckMessage(timestamp_t time);
Message *newBalanceHistoryMessage(BalanceHistory *history, timestamp_t time);
Message *newEmptyBalanceHistoryMessage(void);

#endif
