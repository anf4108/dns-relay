#include "pmhashtable.h"

/*-----------------------------------对外函数-----------------------------------------*/

// 创建PortMap（下简称pm）中的HashTable
pmHashTable * pmHashTable_create(int capacity)
{
	pmHashTable * ht = (pmHashTable *)malloc(sizeof(pmHashTable));
	if (ht == NULL) {
		log_info("[port map]：失败 malloc pmHashTable");
		exit(-1);
	}
	ht->alpha = 0.7;
	ht->tablesize = closestPrime(capacity * 1.0 / ht->alpha);
	ht->cells = (struct pmCell *)malloc(sizeof(struct pmCell) * ht->tablesize);
	if (ht->cells == NULL) {
		log_info("[port map]：失败 malloc pmHashTable's cells");
		exit(-1);
	}

	for (int i = 0; i < ht->tablesize; i++)
		ht->cells[i].state = EMPTY;
	return ht;
}

// 在pmHashTable中找key，把值放入pvalue，如果没找到返回false
bool pmHashTable_find(pmHashTable * ht, pmKEYTYPE key, pmVALUETYPE * pvalue)
{
	int pos = pmHashTable_findin(ht, key, 0);

	// 无结果
	if (ht->cells[pos].state != VALID)
		return false;

	// 有结果
	*pvalue = ht->cells[pos].value;
	return true;
}

// 在pmHashTable中插入key-value对
void pmHashTable_insert(pmHashTable* ht, pmKEYTYPE key, pmVALUETYPE value)
{
	int pos = pmHashTable_findin(ht, key, 1);
	ht->cells[pos].key = key;
	ht->cells[pos].value = value;
	ht->cells[pos].state = VALID;
}

// 在pbHashTable中移除key,如果不存在key返回false
bool pmHashTable_remove(pmHashTable* ht, pmKEYTYPE key)
{
	int pos = pmHashTable_findin(ht, key, 0);
	if (ht->cells[pos].state != VALID) {
		log_info("[错误]在hashtable中删不存在的key");
		return false;
	}
	ht->cells[pos].state = DELETED; //延迟删除
	return true;
}


// 销毁pmHashTable
void pmHashTable_destroy(pmHashTable * ht) {
	free(ht->cells);
	free(ht);
}

/*-----------------------------------内部函数---------------------------------*/


// hash函数
int pmHashFunction(int tablesize, pmKEYTYPE key)
{
	int hash = key.exseq % tablesize;
	for (int i = 0; i < IP4SIZE; i++) {
		hash = 31 * hash + key.ip[i];
		hash = hash % tablesize;
	} 
	return hash % tablesize;
}

// key判等
bool pmEqual(pmKEYTYPE k1, pmKEYTYPE k2)
{
	return (strncmp(k1.ip, k2.ip, IP4SIZE) == 0 && k1.exseq == k2.exseq) ? true : false;
}

// 查找
int pmHashTable_findin(pmHashTable * ht, pmKEYTYPE key, int tag)
{	// tag==1：找空位,返回的index必然是空位
	// tag==0：普通查找，返回的index可能是有效数据代表找到，可能是无效数据代表无

	int currentPos = pmHashFunction(ht->tablesize, key);
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
				|| (ht->cells[newPos].state == VALID && !pmEqual(key, ht->cells[newPos].key));
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
	}
	return newPos;
}