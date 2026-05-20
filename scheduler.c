#include "scheduler.h"
#include "client.h"
#include "queue.h"
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>

extern struct RequestQueue queue;
extern pthread_mutex_t queue_mutex;
extern pthread_cond_t not_empty;
extern pthread_cond_t not_full;

static void disconnect_client(int i) {
    cleanup_client(&clients[i]);
    close(clients[i].fd_read);
    close(clients[i].fd_write);

    char c2s[32], s2c[32];
    snprintf(c2s, sizeof(c2s), "FIFO_C2S_%d", clients[i].pid);
    snprintf(s2c, sizeof(s2c), "FIFO_S2C_%d", clients[i].pid);
    unlink(c2s);
    unlink(s2c);

    pthread_mutex_destroy(&clients[i].order_mutex);
    pthread_cond_destroy(&clients[i].order_cond);

    clients[i] = clients[num_clients - 1];
    num_clients--;
}

void *run_scheduler(void *arg) {
    (void)arg;
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd = 0;

        pthread_mutex_lock(&clients_mutex);
        if (num_clients == 0) {
            pthread_mutex_unlock(&clients_mutex);
            sleep(1);
            continue;
        }
        for (int i = 0; i < num_clients; i++) {
            FD_SET(clients[i].fd_read, &read_fds);
            if (clients[i].fd_read > max_fd) {
                max_fd = clients[i].fd_read;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        pthread_mutex_lock(&clients_mutex);

        // crash detection
        for (int i = 0; i < num_clients; i++) {
            if (kill(clients[i].pid, 0) == -1) {
                disconnect_client(i);
                i--;
            }
        }

        // check which client has data
        for (int i = 0; i < num_clients; i++) {
            if (FD_ISSET(clients[i].fd_read, &read_fds)) {
                char buf[256];
                int n = read(clients[i].fd_read, buf, sizeof(buf) - 1);

                if (n == 0) {
                    disconnect_client(i);
                    i--;

                } else if (n > 0) {
                    buf[n] = '\0';

                    // // DEBUG
                    // printf("DEBUG SCHEDULER: received from pid=%d cmd=%s",
                    //     (int)clients[i].pid, buf);
                    // fflush(stdout);
                    // //

                    struct Request req;
                    strncpy(req.command, buf, sizeof(req.command) - 1);
                    req.command[sizeof(req.command) - 1] = '\0';
                    req.seq = clients[i].next_seq++;
                    req.client = &clients[i];
                    pthread_mutex_unlock(&clients_mutex);

                    pthread_mutex_lock(&queue_mutex);
                    while (isFull(&queue)) {
                        pthread_cond_wait(&not_full, &queue_mutex);
                    }
                    enqueue(&queue, req);
                    pthread_cond_signal(&not_empty);
                    pthread_mutex_unlock(&queue_mutex);

                    pthread_mutex_lock(&clients_mutex);
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}