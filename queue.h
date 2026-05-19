#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

#define QUEUE_SIZE 1000

struct Client;

struct Request {
    char command[256];
    int seq;
    struct Client *client;
};

struct RequestQueue {
    struct Request items[QUEUE_SIZE];
    int front;
    int size;
};

struct RequestQueue *initializeQueue(struct RequestQueue *q);
bool isEmpty(struct RequestQueue *q);
bool isFull(struct RequestQueue *q);
void enqueue(struct RequestQueue *q, struct Request r);
struct Request dequeue(struct RequestQueue *q);

#endif