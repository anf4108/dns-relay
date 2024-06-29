# ifndef PORTMAP_H
# define PORTMAP_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "utils.h"

#define IP4SIZE 4


struct content {
	char ip[IP4SIZE]; 
	uint16_t local_seq; //客户端的查询序号
};

typedef struct {
	uint16_t current_seq;		//当前已经分配的序列号
	int size;					//总容量
	struct content * contents;	//数组
} PortMap;


PortMap * PortMap_create(int capacity);
bool PortMap_allocSeq(PortMap * pm, char ip[IP4SIZE], uint16_t local_seq, uint16_t * global_seq);
bool PortMap_querySeq(PortMap * pm, uint16_t global_seq, uint16_t * local_seq, char ip[IP4SIZE]);
void PortMap_destroy(PortMap * pm);

#endif