#include "network.h"


// Close a socket pipe
int CloseSocket(int sockFd) {
  int err;

  err = close(sockFd);
  if(err < 0) {
    ErrorHandler(__FILE__, __FUNCTION__, __LINE__-2, 
      "Server failed to accept an incoming connection from a client");
    return -1;
  }
}