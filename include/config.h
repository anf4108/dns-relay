#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include "bufferpool.h"
#include "portmap.h"

// TODO: 接收参数时处理宏
// #define LOG_MASK 15

extern const int BUFFER_SIZE;
extern const int IPSIZE;
extern const int PORT;
extern const char *SERVER_IPADDR;
extern const char *LOCAL_IPADDR;
extern BufferPool *bp;
extern PortMap *pm;

#endif