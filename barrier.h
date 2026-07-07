#ifndef BARRIER_H
#define BARRIER_H

#include <pthread.h>
#include "client.h"

#define MAX_BARRIERS 100

struct CanvasBarrier {
    struct canvas *cv;
    int total;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

extern struct CanvasBarrier barriers[MAX_BARRIERS];
extern int num_barriers;
extern pthread_mutex_t barriers_mutex;

struct CanvasBarrier *find_or_create_barrier(struct canvas *cv);

void barrier_add_client(struct canvas *cv);

void barrier_remove_client(struct canvas *cv);

void barrier_wait(struct canvas *cv, char *response);

#endif