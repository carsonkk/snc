#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "global.h"
#include "communication.h"

typedef struct {
  int sock_fd;
  int isUdp;
  struct sockaddr_in *udp_addr;
  socklen_t *udp_addr_len;
} server_t;

int Server(options_t opts);

#endif
