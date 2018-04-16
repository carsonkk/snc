#include "client.h"

/*
 * Spawns a netcat instance as a client
 */
int Client(options_t opts) {
  int err;
  int serverFd;
  int clientFd;
  int sendRet;
  int recvRet;
  int optVal = 1;
  struct sockaddr_in serverAddr;
  struct sockaddr_in clientAddr;
  socklen_t serverAddrLen = sizeof(serverAddr);
  socklen_t clientAddrLen = sizeof(clientAddr);
  socklen_t optLen;
  comm_args_t *commArgs;
  comm_rets_t *commRets;

  // Open a socket
  clientFd = OpenSocket(opts->domain, opts->type, opts->protocol);
  if(clientFd == -1) {
    return -1;
  }

  // Configure the socket of the expected server socket
  err = ConfigureSocket(&serverAddr, &serverAddrLen, opts->hostname, opts->port, opts->domain, opts->type);
  if(err == -1) {
    return -1;
  }

  // Configure the socket of the expected server socket
  if(opts->sourceAddr != NULL) {
    err = ConfigureSocket(&clientAddr, &clientAddrLen, opts->sourceAddr, 0, opts->domain, opts->type);
    if(err == -1) {
      return -1;
    }
  }

  // Bind to a port on the client
  err = BindSocket(clientFd, &clientAddr, &clientAddrLen);
  if(err == -1) {
    return -1;
  }

  // Connect to the server
  if(opts->protocol == IPPROTO_TCP) {
    err = InitiateConnection(clientFd, &serverAddr, &serverAddrLen);
    if(err == -1) {
      return -1;
    }
  }

  // Spawn and join communication threads
  commArgs = (comm_args_t)malloc(sizeof(comm_args_t));
  commArgs->sockFd = clientFd;
  commArgs->protocol = opts->protocol;
  commArgs->udpAddr = (struct sockaddr *)&serverAddr;
  commArgs->udpAddrLen = &serverAddrLen;

  err = pthread_create(&sendId, NULL, SendThread, (void *)commArgs);
  if(err != 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Client failed to spawn the \"send\" communication thread");
    return -1;
  }
  err = pthread_create(&recvId, NULL, RecvThread, (void *)commArgs);
  if(err != 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Client failed to spawn the \"receive\" communication thread");
    return -1;
  }

  err = pthread_join(sendId, (void **)&commRets);
  sendRet = commRets->err;
  free(commRets);
  if(err != 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Client failed to join the \"send\" communication thread");
    return -1;
  }
  // TODO: Check connection end state here
  if(sendRet != 0) {
    pthread_cancel(recvId);
  }
  else {
    err = pthread_join(recvId, (void **)&commRets);
    recvRet = commRets->err;
    free(commRets);
    if(err != 0) {
      ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
        "Client failed to join the \"receive\" communication thread");
      return -1;
    }
  }
  // TODO: error condition already printed fromw ithin thread?
  if(sendRet != 0 || recvRet != 0) {
    return -1;
  }
  
  // Close socket connection
  err = CloseSocket(clientFd);
  if(err == -1) {
    return -1;
  }

  return 0;
}
