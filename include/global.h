#ifndef GLOBAL_H
#define GLOBAL_H

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

typedef struct {
  int domain;
  int type;
  int protocol;
  int port;
  char *hostname;
  char *sourceAddr;
} options_t;

void ErrorHandler(char *fileName, char *functionName, char *lineNum, char *details);

#endif
