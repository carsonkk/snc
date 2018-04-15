#include "global.h"


void ErrorHandler(char *fileName, char *functionName, int lineNum, char *details) {
  int err = errno;

  fprintf(stderr, "==================================================\n");
  fprintf(stderr, "From %s in %s() on line %d:\n", fileName, functionName, lineNum);
  fprintf(stderr, "\tERRNO:  %d\n", err);
  fprintf(stderr, "\tDESC:   %s\n", strerror(err));
  fprintf(stderr, "\tDETAIL: %s\n", details);
  fprintf(stderr, "==================================================\n");
}