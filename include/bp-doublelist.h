#ifndef DOUBLELIST_H
#define DOUBLELIST_H

#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "tools.h"
#include "utils.h"
// #include "config.h"
// #define LOG_MASK 15

#define IP4SIZE 4
#define IP6SIZE 16

typedef struct item bpDATATYPE;
typedef struct domainAndClass bpKEYTYPE; //域名+ip版本号


struct domainAndClass {
	char * domain;
	bool isIpv4;
};

struct item {
	bpKEYTYPE key;	
	char ip[IP6SIZE]; //究竟是ipv4还是ipv6地址在key中有写
	time_t deadTime; //如果该字段值为INT64_MAX，代表这个数据是永久的（即从txt读入的那些数据）
};

struct Node {
	bpDATATYPE data;
	struct Node * pre, *nxt;
};

typedef struct {
	struct Node * head;
	struct Node * tail;
} DoubleList;

// 创建双向链表
DoubleList * DoubleList_create();

// 删除双向链表dl中的第一个元素
bpDATATYPE DoubleList_deleteFirst(DoubleList * dl);

// 将双向链表dl中的节点p移到末尾
void DoubleList_moveToLast(DoubleList * dl, struct Node * p);

// 将数据data插入双向链表dl的末尾
struct Node * DoubleList_insertAsLast(DoubleList * dl, bpDATATYPE * data);

// 销毁双向链表
void DoubleList_destroy(DoubleList * dl);

// 输出双向链表（调试用）
void DoubleList_print(DoubleList * dl);

// 移除某节点
bpDATATYPE DoubleList_remove(DoubleList * dl, struct Node * p);

// 深拷贝数据
void DoubleList_copyData(bpDATATYPE * d1, bpDATATYPE * d2);

// 销毁某节点
void DoubleList_deleteNode(struct Node * p);

// 找下一个过时的项
void DoubleList_findNextTTLOut(DoubleList * dl, struct Node ** p);

#endif