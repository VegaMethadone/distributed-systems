#include "queue.h"

/*

    первое событие - переход из некретической секции в критическую секцию. (REQUEST_CS)
        1. отправить всем реквест с моим таймстепом
        2. кладу реквест в очередь с моим таймстепом

    второе событие - полуние этого запроса другим запросом
        1. положитб запрос к себе в очередь  и отсортировать в порядке возрастания
        2. отправит реплай со своей временной отметкой.

    
    Критерии входа в рит секцию
        1. В моей очередилежит запрос и он имеет наименьшую отмекту лемпорта среди всех запросов в очереди
        2. Получил все сообщения с таймпстемпаи от других процессов, где таймстепмы больше, чем мой таймстемп 

    Выход из критической секции
        1. Удаляет свой запрос из своей очереди
        2. отправляет сообщение релиз всем другим процессам
        3. другие процессы после сообщения релиз удаляют из своих очередей

*/




queue *newQueue(void) {
    queue *newQ = malloc(sizeof(queue));
    newQ->arr = malloc(sizeof(pair) * 1024);
    newQ->len = 0;

    return newQ;
}

pair newPair(local_id id, timestamp_t time) {
    return (pair){id, time};
}


// void siftUp(queue *q) {
//     int i = q->len-1;
//     while (q->arr[i].time < q->arr[(i - 1) / 2].time) {
//         pair tmp = q->arr[(i - 1) / 2];
//         q->arr[(i - 1) / 2] = q->arr[i];
//         q->arr[i] = tmp;

//         i = (i - 1) / 2;
//     }
// }

// void siftDown(queue *q) {
//     int i = 0;

//     while (2 * i + 1 < q->len) {
//         int left = 2*i+1;
//         int right = 2*i+2;
//         int j = left;

//         if (right < q->len && (q->arr[right].time < q->arr[left].time)) {
//             j = right;  
//         }
//         if (q->arr[i].time <= q->arr[j].time) {
//             break;
//         }

//         pair tmp = q->arr[i];
//         q->arr[i] = q->arr[j];
//         q->arr[j] = tmp;

//         i = j;

//     }
// }

void siftUp(queue *q) {
    int i = q->len - 1;
    while (i > 0) {
        int parent = (i - 1) / 2;
        
        if (q->arr[i].time < q->arr[parent].time || 
            (q->arr[i].time == q->arr[parent].time && q->arr[i].id < q->arr[parent].id)) {

            pair tmp = q->arr[parent];
            q->arr[parent] = q->arr[i];
            q->arr[i] = tmp;

            i = parent;
        } else {
            break;
        }
    }
}

void siftDown(queue *q) {
    int i = 0;

    while (2 * i + 1 < q->len) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int j = left;

        if (right < q->len && (q->arr[right].time < q->arr[left].time || 
            (q->arr[right].time == q->arr[left].time && q->arr[right].id < q->arr[left].id))) {
            j = right;
        }

        if (q->arr[i].time < q->arr[j].time || 
            (q->arr[i].time == q->arr[j].time && q->arr[i].id < q->arr[j].id)) {
            break;
        }

        pair tmp = q->arr[i];
        q->arr[i] = q->arr[j];
        q->arr[j] = tmp;

        i = j;
    }
}


void push(queue *q, pair p) {
    if (q->len == 0) {
        q->arr[0] = p;
        q->len++;
        return;
    }

    q->arr[q->len] = p;
    q->len++;

    siftUp(q);
}

pair getMin(queue *q) {
    if (q->len == 0) {
        printf("No value in queue\n");
        return (pair){-1, -1};
    }

    return q->arr[0];
}


pair pop(queue *q) {
    pair minimal = getMin(q);
    q->arr[0] = q->arr[q->len-1];
    q->len--;
    
    siftDown(q);

    return minimal;
}


void printQueue(queue *q) {
    int i = 0;
    
    if (q->len != 0 ) {
        printf("[%d, %d]\t", q->arr[i].id, q->arr->time);
    }

    while (i*2 + 1 < q->len) {
        printf("[%d, %d]\t", q->arr[i*2+1].id, q->arr[i*2+1].time);


        if (i*2 + 2 < q->len) {
            printf("[%d, %d]\t", q->arr[i*2+2].id, q->arr[i*2+2].time);
        }
        
        i++;
    }

    printf("\n");
}
