#include "client.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
struct Client clients[MAX_CLIENTS];
int num_clients = 0;

void add_client(pid_t cpid, int fd_read, int fd_write) {
    pthread_mutex_lock(&clients_mutex);
    clients[num_clients].pid = cpid;
    clients[num_clients].fd_read = fd_read;
    clients[num_clients].fd_write = fd_write;
    clients[num_clients].next_seq = 0;
    clients[num_clients].next_to_send = 0;
    clients[num_clients].logged_in = 0;
    clients[num_clients].username[0] = '\0';
    clients[num_clients].num_canvases = 0;
    clients[num_clients].num_sprites = 0;
    clients[num_clients].num_placements = 0;
    pthread_mutex_init(&clients[num_clients].order_mutex, NULL);
    pthread_cond_init(&clients[num_clients].order_cond, NULL);
    num_clients++;
    pthread_mutex_unlock(&clients_mutex);
}

void handle_login(struct Client *client, char *username, char *response) {
    FILE *f = fopen("users.txt", "r");
    if (f == NULL) {
        snprintf(response, 256, "Reject UNAUTHORISED\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        char *name = strtok(line, " \t");
        char *bal  = strtok(NULL, " \t");
        if (name == NULL || bal == NULL) continue;

        if (strcmp(name, username) == 0) {
            int balance = atoi(bal);
            if (balance > 0) {
                client->logged_in = 1;
                strncpy(client->username, username, 32);
                client->username[32] = '\0';
                snprintf(response, 256, "%d\n", balance);
            } else {
                snprintf(response, 256, "Reject BALANCE\n");
            }
            fclose(f);
            return;
        }
    }
    fclose(f);
    snprintf(response, 256, "Reject UNAUTHORISED\n");
}