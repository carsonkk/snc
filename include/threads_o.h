#ifndef THREADS_H
#define THREADS_H

/*includes*/
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

/*structs*/
struct arg_struct
{
	int sock_fd;
	int udp;
	struct sockaddr_in *udp_addr;
	socklen_t *udp_addr_len;
};
struct ret_struct
{
	int ret;
};

/*variables*/
char *err_msg;
int end_conn;

/*functions*/
void *send_thread(void *arg);
void *recv_thread(void *arg);

#endif