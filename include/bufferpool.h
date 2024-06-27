#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H

#include "bp-hashtable.h"

typedef struct {
	time_t nextRefreshTime;		//下次刷新时间
	int refreshDeltaTime;		//刷新间隔
	int capacity;				//缓存总容量
	int currentNum;				//当前存了多少数据
	DoubleList * dl;			//双向链表
	bpHashTable * ht;			//哈希表（"key->节点指针"的映射）
} BufferPool;


BufferPool * BufferPool_create(int capacity, int refreshDeltaTime);
bool BufferPool_find(BufferPool * bp, char * domain, bool isIPv4, char ipaddr[IP6SIZE]);
void BufferPool_add(BufferPool * bp, char * domain, bool isIPv4, uint32_t ttl, char ipaddr[IP6SIZE]);
void BufferPool_destroy(BufferPool * bp);

#endif
