#include "server.h"

#define SERVICE "http"
#define MAX_BACKLOG 512


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
  serverFd = socket(opts->domain, opts->type, opts->protocol);
  if(serverFd < 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Server failed to open a new socket file descriptor");
    return -1;
  }

  // Allow immediate address reuse
  err = setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optVal , optLen);

  // Configure the socket as a server socket
  memset((char *)&serverAddr, 0, serverAddrLen);
  serverAddr.sin_family = opts->domain;
  serverAddr.sin_port = htons(opts->port);
  if(opts->hostname == NULL) {
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  else {
    serverAddr.sin_addr.s_addr = inet_addr(opts->hostname);

    // If this initially fails, attempt to resolve
    if(serverAddr.sin_addr.s_addr == -1) {
      struct addrinfo *infoHints;
      struct addrinfo **infoRes;
      struct addrinfo **i;
      struct sockaddr_in *hostAddr;
      char *newHostname;

      memset(&infoList, 0, sizeof(infoList));
      infoList.ai_family = opts->domain;
      infoList.ai_socktype = opts->type;
      err = getaddrinfo(opts->hostname, SERVICE, infoHints, infoRes);
      if(err != 0) {
        ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
          "Server failed while attempting to resolve the hostname");
        return -1;
      }
      for(i = infoRes; i != NULL; i = i->ai_next) {
        hostAddr = (struct sockaddr_in *)i->ai_addr;
        newHostname = &(inet_ntoa(hostAddr->sin_addr));
      }
      serverAddr.sin_addr.s_addr = inet_addr(newHostname);
      freeaddrinfo(infoRes);
      // TODO: Error handling needed here?
    }
  }

  // Bind to a port on the server
  err = bind(serverFd, (struct sockaddr *)&serverAddr, &serverAddrLen);
  if(err != 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Server failed to bind the chosen port");
    return -1;
  }

  // Listen for and accept any connections
  if(opts->protocol == IPPROTO_TCP) {
    err = listen(serverFd, MAX_BACKLOG);
    if(err != 0) {
      ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
        "Server failed to listen for any incoming connections");
      return -1;
    }
    clientFd = accept(serverFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if(clientFd < 0) {
      ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
        "Server failed to accept an incoming connection from a client");
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
    err = close(clientFd);
  }
  else {
    err = close(serverFd);
  }
  if(err < 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Server failed to close the socket");
    return -1;
  }

  return 0;
}
