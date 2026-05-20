#define _POSIX_C_SOURCE 200809L

#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static volatile sig_atomic_t got_sigusr2 = 0;

void sigusr2_handler(int sig) {
    (void)sig;
    got_sigusr2 = 1;
}

int main(int argc, char *argv[]) {

    if (argc != 2) { 
        fprintf(stderr, "Not correct argument\n");
        return 1;
    }

    pid_t server_pid = atoi(argv[1]);
    if (server_pid <= 0) {
        fprintf(stderr, "Invalid server id\n");
        return 1;
    }

    signal(SIGUSR2, sigusr2_handler);

    if (kill(server_pid, SIGUSR1) == -1) {
        perror("kill failed - is the server running?");
        return 1;
    }

    while (!got_sigusr2) {
        pause();
    }

    // open FIFOs
    pid_t my_pid = getpid();
    char c2s[32], s2c[32];
    snprintf(c2s, sizeof(c2s), "FIFO_C2S_%d", my_pid);
    snprintf(s2c, sizeof(s2c), "FIFO_S2C_%d", my_pid);

    int fd_write = open(c2s, O_WRONLY);
    int fd_read  = open(s2c, O_RDONLY);


    char writebuf[256];
    char readbuf[256];
    int logged_in = 0;
    char username[33] = {0};

    while (1) {
        // read from stdin
        if (fgets(writebuf, sizeof(writebuf), stdin) == NULL) {
            break;
        }

        // strip newline for parsing
        writebuf[strcspn(writebuf, "\n")] = '\0';

        // copy for parsing (strtok modifies string)
        char copy[256];
        strncpy(copy, writebuf, sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = '\0';
        char *cmd = strtok(copy, " ");

        if (cmd == NULL) {
            continue;
        }

        if (strcmp(cmd, "Login") == 0) {
            // get username from command
            char *uname = strtok(NULL, " ");
            if (uname == NULL) {
                printf("Usage: Login <username>\n");
                fflush(stdout);
                continue;
            }

            // send to server with newline
            char to_send[257];
            snprintf(to_send, sizeof(to_send), "%s\n", writebuf);
            write(fd_write, to_send, strlen(to_send));

            // read response
            int n = read(fd_read, readbuf, sizeof(readbuf) - 1);
            readbuf[n] = '\0';
            readbuf[strcspn(readbuf, "\n")] = '\0';

            // check if response is a number (balance)
            char *endptr;
            long balance = strtol(readbuf, &endptr, 10);
            if (*endptr == '\0' && balance > 0) {
                // success
                logged_in = 1;
                strncpy(username, uname, 32);
                username[32] = '\0';
                printf("Welcome %s. Your balance is %ld\n", username, balance);
                fflush(stdout);
            } else {
                // reject
                printf("%s\n", readbuf);
                fflush(stdout);
                break;
            }

        } else if (!logged_in) {
            printf("Not logged in\n");
            fflush(stdout);

        } else if (strcmp(cmd, "Disconnect") == 0) {
            // send disconnect to server
            char to_send[257];
            snprintf(to_send, sizeof(to_send), "%s\n", writebuf);
            write(fd_write, to_send, strlen(to_send));
            break;

        } else {
            // send RPC to server
            char to_send[257];
            snprintf(to_send, sizeof(to_send), "%s\n", writebuf);
            write(fd_write, to_send, strlen(to_send));

            // read response
            int n = read(fd_read, readbuf, sizeof(readbuf) - 1);
            readbuf[n] = '\0';
            readbuf[strcspn(readbuf, "\n")] = '\0';

            // print response based on first token
            char resp_copy[256];
            strncpy(resp_copy, readbuf, sizeof(resp_copy) - 1);
            char *code = strtok(resp_copy, " ");
            char *value = strtok(NULL, " ");

            if (strcmp(code, "-1") == 0) {
                printf("RPC Failed\n");
                fflush(stdout);
            } else if (strcmp(code, "-2") == 0) {
                printf("Value error\n");
                fflush(stdout);
            } else if (strcmp(code, "-3") == 0) {
                printf("Internal error\n");
                fflush(stdout);
            } else if (strcmp(code, "0") == 0) {
                if (value == NULL) {
                    printf("Success\n");
                    fflush(stdout);
                } else {
                    printf("Success %s\n", value);
                    fflush(stdout);
                }
            }
        }
    }

    close(fd_write);
    close(fd_read);

    return 0;
}