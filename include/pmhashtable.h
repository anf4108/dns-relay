#ifndef PMHASHTABLE_H
#define PMHASHTABLE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "tools.h"


#include "utils.h"
// #define LOG_MASK 0x0F

#define IP4SIZE 4
#define IP6SIZE 16

typedef uint16_t pmVALUETYPE;
typedef struct IPandPort pmKEYTYPE;


typedef struct {
	int tablesize; //表长
	double alpha;
	struct pmCell * cells;
} pmHashTable;

struct IPandPort {
	char ip[IP4SIZE]; //源主机的地址（dns报文是用ipv4封装的）
	int exseq; //外部序列号
};

struct pmCell {
	pmKEYTYPE key;
	pmVALUETYPE value;
	CellState state;
};


/*-----------------------------------对外函数---------------------------------*/

// 创建PortMap（下简称pm）中的HashTable
pmHashTable * pmHashTable_create(int capacity);

// 在pmHashTable中找key，把值放入pvalue，如果没找到返回false
bool pmHashTable_find(pmHashTable * ht, pmKEYTYPE key, pmVALUETYPE * pvalue);

// 在pmHashTable中插入key-value对
void pmHashTable_insert(pmHashTable* ht, pmKEYTYPE key, pmVALUETYPE value);

// 在pbHashTable中移除key,如果不存在key返回false
bool pmHashTable_remove(pmHashTable* ht, pmKEYTYPE key);

// 销毁pmHashTable
void pmHashTable_destroy(pmHashTable * ht);

/*-----------------------------------内部函数---------------------------------*/

int pmHashTable_findin(pmHashTable * ht, pmKEYTYPE key, int tag);
int pmHashFunction(int tablesize, pmKEYTYPE key);
bool pmEqual(pmKEYTYPE k1, pmKEYTYPE k2);


#endif