#include "barrier.h"
#include <string.h>
#include <stdio.h>

struct CanvasBarrier barriers[MAX_BARRIERS];
int num_barriers = 0;
pthread_mutex_t barriers_mutex = PTHREAD_MUTEX_INITIALIZER;

struct CanvasBarrier *find_or_create_barrier(struct canvas *cv) {
    for (int i = 0; i < num_barriers; i++) {
        if (barriers[i].cv == cv) return &barriers[i];
    }
    // create new
    struct CanvasBarrier *b = &barriers[num_barriers++];
    b->cv = cv;
    b->total = 0;
    b->count = 0;
    pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->cond, NULL);
    return b;
}

void barrier_add_client(struct canvas *cv) {
    pthread_mutex_lock(&barriers_mutex);
    struct CanvasBarrier *b = find_or_create_barrier(cv);
    b->total++;
    pthread_mutex_unlock(&barriers_mutex);
}

void barrier_remove_client(struct canvas *cv) {
    pthread_mutex_lock(&barriers_mutex);
    for (int i = 0; i < num_barriers; i++) {
        if (barriers[i].cv == cv) {
            barriers[i].total--;
            // wake everyone if all remaining hit barrier
            if (barriers[i].count >= barriers[i].total) {
                barriers[i].count = 0;
                pthread_cond_broadcast(&barriers[i].cond);
            }
            break;
        }
    }
    pthread_mutex_unlock(&barriers_mutex);
}

void barrier_wait(struct canvas *cv, char *response) {
    pthread_mutex_lock(&barriers_mutex);
    struct CanvasBarrier *b = NULL;
    for (int i = 0; i < num_barriers; i++) {
        if (barriers[i].cv == cv) {
            b = &barriers[i];
            break;
        }
    }
    pthread_mutex_unlock(&barriers_mutex);

    if (b == NULL) {
        snprintf(response, 256, "-2\n");
        return;
    }

    pthread_mutex_lock(&b->mutex);
    b->count++;
    if (b->count >= b->total) {
        // everyone hit barrier - wake all
        b->count = 0;
        pthread_cond_broadcast(&b->cond);
    } else {
        // wait for others
        pthread_cond_wait(&b->cond, &b->mutex);
    }
    pthread_mutex_unlock(&b->mutex);

    snprintf(response, 256, "0\n");
}