#ifndef COMMUNICATION_H
#define COMMUNICATION_H

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

typedef struct {
  int sockFd;
  int protocol;
  struct sockaddr *udpAddr;
  socklen_t *udpAddrLen;
} comm_args_t;
typedef struct {
  int err;
} comm_rets_t;

void *SendThread(void *args);
void *RecvThread(void *args);

#endif
