#include <stdlib.h>
#include <stdio.h>

#include "msg.h"
#include "pa2345.h"
#include "common.h"


typedef struct pair pair;
typedef struct queue queue;

struct pair {
    local_id id;
    timestamp_t time;
};

struct queue {
    pair *arr;
    int len;
};



queue *newQueue(void);
void siftUp(queue *q);
void siftDown(queue *q);
void push(queue *q, pair p);
pair getMin(queue *q);
pair pop(queue *q);
void printQueue(queue *q);
pair newPair(local_id id, timestamp_t time);
