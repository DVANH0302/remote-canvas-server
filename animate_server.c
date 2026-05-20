#define _POSIX_C_SOURCE 200809L

#include "client.h"
#include "queue.h"
#include "worker.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

struct RequestQueue queue;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

static volatile sig_atomic_t new_client_pid = 0;

void sigusr1_handler(int sig, siginfo_t *info, void *context) {
    (void)sig; (void)context;
    pid_t client_pid = info->si_pid;

    char c2s[32], s2c[32];
    snprintf(c2s, sizeof(c2s), "FIFO_C2S_%d", client_pid);
    snprintf(s2c, sizeof(s2c), "FIFO_S2C_%d", client_pid);

    unlink(c2s);
    unlink(s2c);
    mkfifo(c2s, 0666);
    mkfifo(s2c, 0666);

    if (kill(client_pid, SIGUSR2) == -1) {
        perror("kill failed");
        return;
    }
    new_client_pid = client_pid;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Not correct argument\n");
        return 1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Invalid number of threads\n");
        return 1;
    }

    printf("Server PID: %d\n", getpid());
    fflush(stdout);

    struct sigaction sa;
    sa.sa_sigaction = sigusr1_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    initializeQueue(&queue);

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    pthread_t scheduler;
    pthread_create(&scheduler, NULL, run_scheduler, NULL);

    while (1) {
        pause();
        while (new_client_pid != 0) {
            pid_t cpid = new_client_pid;
            new_client_pid = 0;

            char c2s[32], s2c[32];
            snprintf(c2s, sizeof(c2s), "FIFO_C2S_%d", cpid);
            snprintf(s2c, sizeof(s2c), "FIFO_S2C_%d", cpid);

            int fd_read  = open(c2s, O_RDONLY | O_NONBLOCK);
            int fd_write = open(s2c, O_WRONLY);
            add_client(cpid, fd_read, fd_write);
        }
    }

    free(threads);
    return 0;
}