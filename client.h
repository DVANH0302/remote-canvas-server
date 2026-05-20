#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>
#include <sys/types.h>
#include <animate/animate.h>

#define MAX_CLIENTS 100

struct Client {
    pid_t pid;
    int fd_read;
    int fd_write;
    int next_seq;
    int next_to_send;
    int logged_in;
    char username[33];
    pthread_mutex_t order_mutex;
    pthread_cond_t order_cond;
    struct canvas *canvases[100];
    size_t canvas_widths[100];
    size_t canvas_heights[100];
    
    struct sprite *sprites[100];
    struct sprite_placement *placements[100];
    int num_canvases;
    int num_sprites;
    int num_placements;

};

extern struct Client clients[MAX_CLIENTS];
extern int num_clients;
extern pthread_mutex_t clients_mutex;

void add_client(pid_t cpid, int fd_read, int fd_write);
void cleanup_client(struct Client *client);
void handle_login(struct Client *client, char *username, char *response);

#endif