#include "network.h"

int OpenSocket(int domain, int type, int protocol) {
  int err;

  err = socket(domain, type, protocol);
  if(err < 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Failed to open a new socket file descriptor");
    return -1;
  }
  
  return 0;
}

int ConfigureSocket(struct sockaddr_in *addr, socklen_t *addrLen, char *hostname, int port, int domain, int type) {
  int err;
  char *service = "http";

  memset((char *)addr, 0, *addrLen);
  addr->sin_family = domain;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = inet_addr(hostname);

  // If the hostname isn't understood in its current format, attempt to resolve
  if(addr.sin_addr.s_addr == -1) {
    struct addrinfo *infoHints;
    struct addrinfo **infoRes;
    struct addrinfo **i;
    struct sockaddr_in *hostAddr;
    char *newHostname;

    memset(&infoList, 0, sizeof(infoList));
    infoList.ai_family = domain;
    infoList.ai_socktype = type;
    err = getaddrinfo(addr, service, infoHints, infoRes);
    if(err != 0) {
      ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
        "Failed while attempting to resolve the source address's hostname");
      return -1;
    }
    for(i = infoRes; i != NULL; i = i->ai_next) {
      addr = (struct sockaddr_in *)i->ai_addr;
      newHostname = &(inet_ntoa(addr->sin_addr));
    }
    addr.sin_addr.s_addr = inet_addr(newHostname);
    freeaddrinfo(infoRes);
    // TODO: Error handling needed here?
  }

  return 0;
}

int BindSocket(int fd, struct sockaddr_in *addr, socklen_t *addrLen) {
  int err;

  err = bind(fd, (struct sockaddr *)addr, addrLen);
  if(err != 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Failed to bind the chosen port");
    return -1;
  }

  return 0;
}

int InitiateConnection(int clientFd, struct sockaddr_in *serverAddr, socklen_t *serverAddrLen) {
  int err;

  err = connect(clientFd, (struct sockaddr *)serverAddr, serverAddrLen);
  if(err != 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Client failed to connect to the specified server");
    return -1;
  }

  return 0;
}

int AcceptConnection(int serverFd, struct sockaddr_in *clientAddr, socklen_t *clientAddrLen) {
  int err;
  int clientFd;
  int maxBacklog = 512;

  err = listen(serverFd, maxBacklog);
  if(err != 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Server failed to listen for any incoming connections");
    return -1;
  }
  clientFd = accept(serverFd, (struct sockaddr *)clientAddr, clientAddrLen);
  if(clientFd < 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Server failed to accept an incoming connection from a client");
    return -1;
  }

  return clientFd;
}

int CloseSocket(int fd) {
  int err;

  err = close(fd);
  if(err < 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__, 
      "Failed to close the socket");
    return -1;
  }

  return 0;
}