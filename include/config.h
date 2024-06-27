#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include "bufferpool.h"

extern const int BUFFER_SIZE;
extern const int IPSIZE;
extern const int PORT;
extern const char *SERVER_IPADDR;
extern const char *LOCAL_IPADDR;
extern BufferPool *bp;

#endif