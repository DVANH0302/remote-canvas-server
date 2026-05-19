#include "queue.h"

struct RequestQueue *initializeQueue(struct RequestQueue *q) {
    q->front = 0;
    q->size = 0;
    return q;
}

bool isEmpty(struct RequestQueue *q) {
    return (q->size == 0);
}

bool isFull(struct RequestQueue *q) {
    return (q->size == QUEUE_SIZE);
}

void enqueue(struct RequestQueue *q, struct Request r) {
    int rear = (q->front + q->size) % QUEUE_SIZE;
    q->items[rear] = r;
    q->size++;
}

struct Request dequeue(struct RequestQueue *q) {
    struct Request r = q->items[q->front];
    q->front = (q->front + 1) % QUEUE_SIZE;
    q->size--;
    return r;
}