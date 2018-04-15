#include "communication.h"

#define BUFFER_SIZE 4096


void *SendThread(void *args) {
  int err;
  char buf[BUFFER_SIZE];
  comm_args_t *targs = (comm_args_t *)args;
  comm_rets_t *trets = (comm_rets_t *)malloc(sizeof(comm_rets_t));

  while(1) {

    memset(buf, 0, BUFFER_SIZE);

    if(fgets(buf, BUFFER_SIZE, stdin) == NULL) {
      trets->err = 0;
      if(targs->protocol == IPPROTO_TCP) {
        send(targs->sockFd, buf, BUFFER_SIZE, 0);
      }
      break;
    }

    if(targs->protocol == IPPROTO_TCP) {
      err = send(targs->sockFd, buf, BUFFER_SIZE, 0);
    }
    else {
      err = sendto(targs->sockFd, buf, BUFFER_SIZE, 0, targs->udpAddr, targs->udpAddrLen);
    }

    if(err != BUFFER_SIZE) {
      if(targs->protocol == IPPROTO_UDP && errno == EAFNOSUPPORT) {
        // TODO: error handling?
        fprintf(stderr, "UDP eror?\n");
      }
      else {
        fprintf(stderr, "Something else?\n");
        trets->err = -1;
        break;
      }
    }
  }

  return (void *)trets;
}

void *RecvThread(void *args) {
  int err;
  char buf[BUFFER_SIZE];
  char emptyBuf[BUFFER_SIZE];
  comm_args_t *targs = (comm_args_t *)args;
  comm_rets_t *trets = (comm_rets_t *)malloc(sizeof(comm_rets_t));

  while(1) {

    memset(buf, 0, BUFFER_SIZE);

    if(targs->protocol == IPPROTO_TCP) {
      err = recv(targs->sockFd, buf, BUFFER_SIZE, 0);
    }
    else {
      err = recvfrom(targs->sockFd, buf, BUFFER_SIZE, 0, targs->udpAddr, targs->udpAddrLen);
    }

    // Catch if CTRL^C or CTRL^D were pressed
    if(err == 0 || strcmp(buf, emptyBuf) == 0) {
      trets->err = 0;
      break;
    }

    // Handle error during transmission
    if(err != BUFFER_SIZE) {
      fprintf(stderr, "Something else?\n");
      trets->err = -1;
      break;
    }

    fprintf(stdout, "%s", buf);
  }

  return (void *)trets;
}
