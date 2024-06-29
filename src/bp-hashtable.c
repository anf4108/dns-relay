#include "bp-hashtable.h"

/*-----------------------------------对外函数-----------------------------------------*/

// 创建BufferPool（下简称bp）中的HashTable
bpHashTable * bpHashTable_create(int capacity)
{
	bpHashTable * ht = (bpHashTable *)malloc(sizeof(bpHashTable));
	if (ht == NULL) {
		log_error("[buffer pool]失败malloc bpHashTable");
		exit(-1);
	}
	ht->alpha = 0.7;
	ht->tablesize = closestPrime(capacity * 1.0 / ht->alpha);
	ht->cells = (struct bpCell *)malloc(sizeof(struct bpCell) * ht->tablesize);
	if (ht->cells == NULL) {
		log_error("[buffer pool]失败malloc bpHashTable's cells");
		exit(-1);
	}
	
	for (int i = 0; i < ht->tablesize; i++)
		ht->cells[i].state = EMPTY;
	return ht;
}

// 在bpHashTable中找key，把值放入pvalue，如果没找到返回false
bool bpHashTable_find(bpHashTable * ht, bpKEYTYPE key, bpVALUETYPE * pvalue)
{
	int pos = bpHashTable_findin(ht, key, 0);

	// 无结果
	if (ht->cells[pos].state != VALID)
		return false;

	// 有结果
	*pvalue = ht->cells[pos].value;
	return true;
}

// 在bpHashTable中插入key-value对
void bpHashTable_insert(bpHashTable* ht, bpKEYTYPE key, bpVALUETYPE value)
{
	int pos = bpHashTable_findin(ht, key, 1);
	ht->cells[pos].key = key;
	ht->cells[pos].value = value;
	ht->cells[pos].state = VALID;
}

// 在pbHashTable中移除key,如果不存在key返回false
bool bpHashTable_remove(bpHashTable* ht, bpKEYTYPE key)
{
	int pos = bpHashTable_findin(ht, key, 0);
	if (ht->cells[pos].state != VALID) {
		log_error("[buffer pool]错误：在hashtable中删不存在的key");
		// exit(-1);
	}
	ht->cells[pos].state = DELETED; //延迟删除
	return true;
}



// 销毁bpHashTable
void bpHashTable_destroy(bpHashTable * ht){
	free(ht->cells);
	free(ht);
}

/*-----------------------------------内部函数---------------------------------*/


// hash函数
int bpHashFunction(int tablesize, bpKEYTYPE key)
{
	int hash = key.isIpv4;
	int c;
	while ((c = *(key.domain)++)) {
		hash = 31 * hash + c;
		hash = hash % tablesize;
	}

	return (hash + tablesize) % tablesize;
}

// 域名判等
bool bpEqual(bpKEYTYPE k1, bpKEYTYPE k2)
{
	return (strcmp(k1.domain, k2.domain) == 0 && k1.isIpv4 == k2.isIpv4) ? true : false;
}

// 查找
int bpHashTable_findin(bpHashTable * ht, bpKEYTYPE key, int tag)
{	// tag==1：找空位,返回的index必然是空位
	// tag==0：普通查找，返回的index可能是有效数据代表找到，可能是无效数据代表无

	int currentPos = bpHashFunction(ht->tablesize, key);
	int newPos = currentPos;
	int conflictNum = 0;

	// 平方探测
	while (true) {
		bool execute;
		if (tag == 1) {
			execute = (ht->cells[newPos].state == VALID);
		}
		else if (tag == 0) {
			execute = (ht->cells[newPos].state == DELETED)
				|| (ht->cells[newPos].state == VALID && !bpEqual(key, ht->cells[newPos].key));
		}
		if (!execute) break;

		if (++conflictNum % 2) {
			newPos = currentPos + (conflictNum + 1) * (conflictNum + 1) / 4;
			newPos = (newPos + ht->tablesize) % ht->tablesize;
		}
		else {
			newPos = currentPos + conflictNum * conflictNum / 4;
			newPos = (newPos + ht->tablesize * ht->tablesize) % ht->tablesize;
		}
		if (conflictNum > ht->tablesize) {
			log_error("[buffer pool]hash table 探测table size次仍未找到空位");
			exit(-1);
		}
	}
	return newPos;
}