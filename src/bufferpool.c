#include "bufferpool.h"

BufferPool * BufferPool_create(int capacity, int refreshDeltaTime)
{
	BufferPool * bp = (BufferPool*)malloc(sizeof(BufferPool));
	if (bp == NULL)
	{
		log_info("[buffer pool]失败 malloc Buffer Pool");
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
	log_info("[buffer pool]find函数：当前共%d项", bp->currentNum);
	if (time(NULL) > bp->nextRefreshTime)
	{
		// 整体刷新
		log_info("[buffer pool]开始整体刷新，刷新前共%d项", bp->currentNum);
		
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
		log_info("[buffer pool]整体刷新完毕，刷新后共%d项", bp->currentNum);
	}

	bpKEYTYPE key;
	key.domain = domain;
	key.isIpv4 = isIPv4;

	struct Node * curr;
	log_info("[buffer pool]查找%s的%s地址\n", key.domain, key.isIpv4 ? "IPv4" : "IPv6");

	if (!bpHashTable_find(bp->ht, key, &curr))
	{
		log_info("[buffer pool]未找到\n");
		return false;
	}
		

	if (curr->data.deadTime <= time(NULL))
	{
		log_info("[buffer pool]找到%s但已过时\n", key.domain);
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
	
	log_info("[buffer pool]找到\n");

	return true;
}


void BufferPool_add(BufferPool * bp, char * domain, bool isIPv4, uint32_t ttl, char ipaddr[IP6SIZE])
{
	log_info("[buffer pool]add函数：当前共%d项", bp->currentNum);
	while (bp->currentNum + 1 >= bp->capacity  )
	{
		bpDATATYPE data = DoubleList_deleteFirst(bp->dl);
		bpHashTable_remove(bp->ht, data.key);
		bp->currentNum--;
		log_info("[buffer pool]冲突移出%s\n", data.key.domain);
		
	}
	bp->currentNum++;

	bpDATATYPE data;
	strncpy(data.ip, ipaddr, isIPv4 ? IP4SIZE : IP6SIZE);
	data.deadTime = time(NULL) + ttl;
	data.key.domain = deepCopyStr(domain);
	data.key.isIpv4 = isIPv4;

	struct Node * p = DoubleList_insertAsLast(bp->dl, &data);
	bpHashTable_insert(bp->ht, data.key, p);
	log_info("[buffer pool]：成功加入%s\n", data.key.domain);

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

}


void BufferPool_destroy(BufferPool * bp)
{
	DoubleList_destroy(bp->dl);
	bpHashTable_destroy(bp->ht);
}