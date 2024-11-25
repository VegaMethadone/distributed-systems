#include "channel.h"



// При событиях, где t1 = t2
// Ei < E`j <=> (L(Ei) < L(E`j)) v (L(ei) = L(E`j)) ^ (i < j)

timestamp_t lamport_time = 0;

timestamp_t get_lamport_time(void) {
    return lamport_time;
}

void  add_lamport_time(void) {
    lamport_time++;
}

void max_lamport_time(timestamp_t time) {
    if (time > lamport_time) {
        lamport_time = time;
    }
    lamport_time++;
}
