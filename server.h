#ifndef SERVER_H
#define SERVER_H

/*includes*/
#include "threads.h"

/*functions*/
int server_handler(int udp, char *hostname, int port);

#endif