#include "worker.h"
#include "queue.h"
#include "client.h"
#include "rpc.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

extern struct RequestQueue queue;
extern pthread_mutex_t queue_mutex;
extern pthread_cond_t not_empty;
extern pthread_cond_t not_full;

void *worker(void *arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&queue_mutex);
        while (isEmpty(&queue)) {
            pthread_cond_wait(&not_empty, &queue_mutex);
        }
        struct Request req = dequeue(&queue);
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&queue_mutex);

        char command_copy[256];
        strncpy(command_copy, req.command, sizeof(command_copy) - 1);
        command_copy[sizeof(command_copy) - 1] = '\0';
        command_copy[strcspn(command_copy, "\n")] = '\0';

        char response[256];
        char *cmd = strtok(command_copy, " ");

        if (cmd == NULL) {
            snprintf(response, sizeof(response), "-1\n");
        } else if (strcmp(cmd, "Login") == 0) {
            char *username = strtok(NULL, " ");
            if (username == NULL) {
                snprintf(response, sizeof(response), "Reject UNAUTHORISED\n");
            } else {
                handle_login(req.client, username, response);
            }
        } else if (!req.client->logged_in) {
            snprintf(response, sizeof(response), "Not logged in\n");
        } else if (strcmp(cmd, "Disconnect") == 0) {
            snprintf(response, sizeof(response), "0\n");
        } else {
            handle_rpc(req.client, cmd, response);
        }

        pthread_mutex_lock(&req.client->order_mutex);
        while (req.client->next_to_send != req.seq) {
            pthread_cond_wait(&req.client->order_cond,
                             &req.client->order_mutex);
        }
        // /// DEBUG
        // printf("DEBUG: sending to pid=%d fd_write=%d response=%s", (int)req.client->pid, req.client->fd_write, response);
        // fflush(stdout);
        // ///
        write(req.client->fd_write, response, strlen(response));
        req.client->next_to_send++;
        pthread_cond_broadcast(&req.client->order_cond);
        pthread_mutex_unlock(&req.client->order_mutex);
    }
    return NULL;
}