#ifndef BPHASHTABLE_H
#define BPHASHTABLE_H

#include "bp-doublelist.h"


typedef struct domainAndClass bpKEYTYPE; //域名+ip版本号
typedef struct Node * bpVALUETYPE;


typedef struct {
	int tablesize; //表长
	double alpha;
	struct bpCell * cells;
} bpHashTable;

struct bpCell {
	bpKEYTYPE key;
	bpVALUETYPE value;
	CellState state ;
};


/*-----------------------------------对外函数---------------------------------*/

// 创建BufferPool（下简称bp）中的HashTable
bpHashTable * bpHashTable_create(int capacity);

// 在bpHashTable中找key，把值放入pvalue，如果没找到返回false
bool bpHashTable_find(bpHashTable * ht, bpKEYTYPE key, bpVALUETYPE * pvalue);

// 在bpHashTable中插入key-value对
void bpHashTable_insert(bpHashTable* ht, bpKEYTYPE key, bpVALUETYPE value);

// 在pbHashTable中移除key,如果不存在key返回false
bool bpHashTable_remove(bpHashTable* ht, bpKEYTYPE key);

// 销毁bpHashTable
void bpHashTable_destroy(bpHashTable * ht);

/*-----------------------------------内部函数---------------------------------*/

int bpHashTable_findin(bpHashTable * ht, bpKEYTYPE key, int tag);
int bpHashFunction(int tablesize, bpKEYTYPE key);
bool bpEqual(bpKEYTYPE k1, bpKEYTYPE k2);


#endif