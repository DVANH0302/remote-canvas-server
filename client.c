#include "client.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "barrier.h"

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
    clients[num_clients].canvas_widths[0] = 0;
    clients[num_clients].canvas_heights[0] = 0;
    clients[num_clients].num_sprites = 0;
    clients[num_clients].num_placements = 0;
    pthread_mutex_init(&clients[num_clients].order_mutex, NULL);
    pthread_cond_init(&clients[num_clients].order_cond, NULL);
    num_clients++;
    pthread_mutex_unlock(&clients_mutex);
}


void cleanup_client(struct Client *client) {
    // destroy placements first (before canvas)
    for (int i = 0; i < client->num_placements; i++) {
        if (client->placements[i] != NULL) {
            animate_destroy_placement(client->placements[i]);
            client->placements[i] = NULL;
        }
    }

    // destroy sprites
    for (int i = 0; i < client->num_sprites; i++) {
        if (client->sprites[i] != NULL) {
            animate_destroy_sprite(client->sprites[i]);
            client->sprites[i] = NULL;
        }
    }

    // destroy canvases not shared with others
    for (int i = 0; i < client->num_canvases; i++) {
        if (client->canvases[i] != NULL) {
            int shared = 0;
            for (int j = 0; j < num_clients; j++) {
                if (&clients[j] == client) continue;
                for (int k = 0; k < clients[j].num_canvases; k++) {
                    if (clients[j].canvases[k] == client->canvases[i]) {
                        shared = 1;
                        break;
                    }
                }
                if (shared) break;
            }
            // update barrier regardless
            barrier_remove_client(client->canvases[i]);
            if (!shared) {
                animate_destroy_canvas(client->canvases[i]);
            }
            client->canvases[i] = NULL;
        }
    }

    client->num_canvases = 0;
    client->num_sprites = 0;
    client->num_placements = 0;
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