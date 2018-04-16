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

int OpenSocket(int domain, int type, int protocol);
int ConfigureSocket(struct sockaddr_in *addr, socklen_t *addrLen, char *hostname, int port, int domain, int type);
int BindSocket(int fd, struct sockaddr_in *addr, socklen_t *addrLen);
int InitiateConnection(int clientFd, struct sockaddr_in *serverAddr, socklen_t *serverAddrLen);
int AcceptConnection(int serverFd, struct sockaddr_in *clientAddr, socklen_t *clientAddrLen);

#endif
