# ifndef PORTMAP_H
# define PORTMAP_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pmhashtable.h>
#include "utils.h"

// #define LOG_MASK 0x0F
#define IP4SIZE 4
#define IP6SIZE 16

typedef pmVALUETYPE SEQTYPE;

typedef struct {
	SEQTYPE currentSeq;		//当前已经分配的序列号
	int currentNum;			//当前储存的映射的数量
	pmHashTable * ht;
} PortMap;

PortMap * PortMap_create(int capacity);
bool PortMap_allocSeq(PortMap * pm, char ip[IP4SIZE], SEQTYPE localSeq, SEQTYPE * globalSeq);
bool PortMap_querySeq(PortMap * pm, char ip[IP4SIZE], SEQTYPE globalSeq, SEQTYPE * localSeq);
bool PortMap_remove(PortMap * pm, char ip[IP4SIZE], SEQTYPE globalSeq);
void PortMap_destroy(PortMap * pm);

#endif