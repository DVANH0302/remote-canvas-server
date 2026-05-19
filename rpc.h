#ifndef RPC_H
#define RPC_H

#include "client.h"

void handle_rpc(struct Client *client, char *cmd, char *response);

#endif