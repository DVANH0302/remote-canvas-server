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

// find or create barrier entry for a canvas
struct CanvasBarrier *find_or_create_barrier(struct canvas *cv);

// increment total when share_canvas completes
void barrier_add_client(struct canvas *cv);

// decrement total when client disconnects
void barrier_remove_client(struct canvas *cv);

// called when client hits barrier RPC
void barrier_wait(struct canvas *cv, char *response);

#endif