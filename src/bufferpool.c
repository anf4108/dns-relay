#include "bufferpool.h"

BufferPool * BufferPool_create(int capacity, int refreshDeltaTime)
{
	BufferPool * bp = (BufferPool*)malloc(sizeof(BufferPool));
	if (bp == NULL)
	{
		printf("[失败]malloc Buffer Pool");
		exit(-1);
	}
	bp->nextRefreshTime = time(NULL) + refreshDeltaTime;
	bp->refreshDeltaTime = refreshDeltaTime;
	bp->capacity = capacity;
	bp->dl = DoubleList_create();
	bp->ht = bpHashTable_create(capacity);
	bp->currentNum = 0;
	return bp;
}


bool BufferPool_find(BufferPool * bp, char * domain, bool isIPv4, char ipaddr[IP6SIZE])
{

	if (time(NULL) > bp->nextRefreshTime)
	{
		// 整体刷新
		
		for (struct Node * p = bp->dl->head->nxt; p != bp->dl->tail; DoubleList_findNextTTLOut(bp->dl, &p))
		{
			// 删DoubleList上的该过时节点
			p = p->nxt;
			bpDATATYPE data = DoubleList_remove(bp->dl, p->pre);
			
			// 删bpHashTable中的该过时元素
			bpHashTable_remove(bp->ht, data.key);
			bp->currentNum--;
		}

		bp->nextRefreshTime = time(NULL) + bp->refreshDeltaTime;
	}

	bpKEYTYPE key;
	key.domain = domain;
	key.isIpv4 = isIPv4;

	struct Node * curr;
	printf("[查找]%s的%s地址\n", key.domain, key.isIpv4 ? "IPv4" : "IPv6");

	if (!bpHashTable_find(bp->ht, key, &curr))
	{
		printf("[未找到]\n");
		return false;
	}
		

	if (curr->data.deadTime <= time(NULL))
	{
		printf("[过时]%s\n", key.domain);
		// 表中数据的TTL已到，应该删掉了
		// 删bpHashTable
		bpHashTable_remove(bp->ht, key);
		// 删DoubleList
		DoubleList_remove(bp->dl, curr);
		bp->currentNum--;

		return false;
	}

	// 在双向链表中调整位置
	DoubleList_moveToLast(bp->dl, curr);

	// 写入答案
	strncpy(ipaddr, curr->data.ip, isIPv4 ? IP4SIZE : IP6SIZE);
	
	printf("[找到]\n");


	DoubleList_print(bp->dl);

	return true;
}


void BufferPool_add(BufferPool * bp, char * domain, bool isIPv4, uint32_t ttl, char ipaddr[IP6SIZE])
{
	if (bp->currentNum == bp->capacity)
	{
		bpDATATYPE data = DoubleList_deleteFirst(bp->dl);
		bpHashTable_remove(bp->ht, data.key);
		bp->currentNum--;
		printf("[冲突]：移出%s\n", data.key.domain);
		
	}
	bp->currentNum++;

	bpDATATYPE data;
	strncpy(data.ip, ipaddr, isIPv4 ? IP4SIZE : IP6SIZE);
	data.deadTime = time(NULL) + ttl;
	data.key.domain = deepCopyStr(domain);
	data.key.isIpv4 = isIPv4;

	struct Node * p = DoubleList_insertAsLast(bp->dl, &data);
	bpHashTable_insert(bp->ht, data.key, p);
	printf("[加入]：成功加入%s\n", data.key.domain);

	// 如果加入的信息是IPv4拦截
	if (data.key.isIpv4 &&
		ipaddr[0] == 0 && ipaddr[1] == 0 &&
		ipaddr[2] == 0 && ipaddr[3] == 0)
	{
		// IPv6的拦截也加进去
		data.key.isIpv4 = false;
		memset(data.ip, 0, sizeof(IP6SIZE));
		struct Node * p = DoubleList_insertAsLast(bp->dl, &data);
		bpHashTable_insert(bp->ht, data.key, p);
		bp->currentNum++;
	}

	DoubleList_print(bp->dl);
}




void BufferPool_destroy(BufferPool * bp)
{
	DoubleList_destroy(bp->dl);
	bpHashTable_destroy(bp->ht);
}