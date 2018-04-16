#include "server.h"

/*
 * Spawns a netcat instance as a server
 */
int Server(options_t opts) {
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
  serverFd = OpenSocket(opts->domain, opts->type, opts->protocol);
  if(serverFd == -1) {
    return -1;
  }

  // Allow immediate address reuse
  setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optVal , optLen);

  // Configure the socket as a server socket
  err = ConfigureSocket(&serverAddr, &serverAddrLen, opts->hostname, opts->port, opts->domain, opts->type);
  if(err == -1) {
    return -1;
  }

  // Bind to a port on the server
  err = BindSocket(serverFd, &serverAddr, &serverAddrLen);
  if(err == -1) {
    return -1;
  }

  // Listen for and accept any connections
  if(opts->protocol == IPPROTO_TCP) {
    clientFd = AcceptConnection(serverFd, &clientAddr, &clientAddrLen);
    if(clientFd == -1) {
      return -1;
    }
  }

  // Spawn and join communication threads
  commArgs = (comm_args_t)malloc(sizeof(comm_args_t));
  if(opts->protocol == IPPROTO_TCP) {
    commArgs->sockFd = clientFd;
  }
  else {
    commArgs->sockFd = serverFd;
  }
  commArgs->protocol = opts->protocol;
  commArgs->udpAddr = (struct sockaddr *)&clientAddr;
  commArgs->udpAddrLen = &clientAddrLen;

  err = pthread_create(&sendId, NULL, SendThread, (void *)commArgs);
  if(err != 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Server failed to spawn the \"send\" communication thread");
    return -1;
  }
  err = pthread_create(&recvId, NULL, RecvThread, (void *)commArgs);
  if(err != 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Server failed to spawn the \"receive\" communication thread");
    return -1;
  }

  err = pthread_join(sendId, (void **)&commRets);
  sendRet = commRets->err;
  free(commRets);
  if(err != 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Server failed to join the \"send\" communication thread");
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
        "Server failed to join the \"receive\" communication thread");
      return -1;
    }
  }

  // TODO: error condition already printed fromw ithin thread?
  if(sendRet != 0 || recvRet != 0) {
    return -1;
  }

  // Close socket connection
  if(opts->protocol == IPPROTO_TCP) {
    err = CloseSocket(clientFd);
  }
  else {
    err = CloseSocket(serverFd);
  }
  if(err == -1) {
    return -1;
  }

  return 0;
}
