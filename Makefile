CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread -L libanimate/lib -lanimate

SERVER_SRCS = animate_server.c queue.c client.c rpc.c scheduler.c worker.c
CLIENT_SRCS = animate_client.c

default: animate_server animate_client

animate_server: $(SERVER_SRCS)
	$(CC) $(CFLAGS) $(SERVER_SRCS) -I libanimate/include $(LDFLAGS) -o animate_server

animate_client: $(CLIENT_SRCS)
	$(CC) $(CFLAGS) $(CLIENT_SRCS) -o animate_client

clean:
	rm -f animate_server animate_client FIFO_*

libanimate:
	@if [ ! -d $@ ]; then \
		echo "ERROR: libanimate not found" > /dev/stderr; \
		echo "Download and unzip libanimate.zip from P2 resources" > /dev/stderr; \
		false; \
	fi