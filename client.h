#ifndef CLIENT_H
#define CLIENT_H

/*includes*/
#include "threads.h"

/*functions*/
int client_handler(int udp, char *source, char *hostname, int port);

#endif