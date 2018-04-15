#ifndef GLOBAL_H
#define GLOBAL_H

typedef struct {
  int domain;
  int type;
  int protocol;
  char *hostname;
  int port;
} options_t;

void ErrorHandler(char *fileName, char *functionName, char *lineNum, char *details);

#endif
