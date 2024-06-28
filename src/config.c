#include "config.h"

// #define LOG_MASK 15
const int BUFFER_SIZE = 1024;
const int IPSIZE = 4;
const int PORT = 53;
const char *SERVER_IPADDR = "10.3.9.4";
const char *LOCAL_IPADDR = "0.0.0.0";
FILE *log_file;
BufferPool *bp;
PortMap *pm;