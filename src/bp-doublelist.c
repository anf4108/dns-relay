#include "bp-doublelist.h"

//创建双向链表
DoubleList * DoubleList_create()
{
	DoubleList * dl = (DoubleList *)malloc(sizeof(DoubleList));
	if (dl == NULL) {
		log_info("[buffer pool]失败malloc DoubleList");
		exit(-1);
	}
	dl->head = (struct Node *)malloc(sizeof(struct Node));
	dl->tail = (struct Node *)malloc(sizeof(struct Node));
	if (dl->head == NULL || dl->tail == NULL) {
		log_info("[buffer pool]失败malloc bpHashTable's head or tail");
		exit(-1);
	}
	dl->head->nxt = dl->tail;
	dl->tail->pre = dl->head;
	dl->head->pre = NULL;
	dl->tail->nxt = NULL;

	return dl;
}

// 删除双向链表dl中的第一个元素
bpDATATYPE DoubleList_deleteFirst(DoubleList * dl)
{
	bpDATATYPE data;

	struct Node * curr = dl->head->nxt;
	dl->head->nxt = curr->nxt;
	curr->nxt->pre = dl->head;
	DoubleList_copyData(&data, &(curr->data));
	free(curr);

	return data;
}

// 将双向链表dl中的节点p移到末尾
void DoubleList_moveToLast(DoubleList * dl, struct Node * p)
{
	if (p == dl->tail || p == dl->head) exit(-1);
	if (p == dl->tail->pre) return;

	p->pre->nxt = p->nxt;
	p->nxt->pre = p->pre;
	p->nxt = dl->tail;
	p->pre = dl->tail->pre;
	dl->tail->pre->nxt = p;
	dl->tail->pre = p;
}

// 将数据data插入双向链表dl的末尾
struct Node * DoubleList_insertAsLast(DoubleList * dl, bpDATATYPE * pdata)
{
	struct Node * curr = (struct Node *)malloc(sizeof(struct Node));
	if (curr == NULL) {
		log_info("[buffer pool]失败malloc bpHashTable's new node");
		exit(-1);
	}
	DoubleList_copyData(&(curr->data), pdata);
	curr->pre = dl->tail->pre;
	curr->nxt = dl->tail;
	dl->tail->pre = curr;
	curr->pre->nxt = curr;
	return curr;
}

// 销毁双向链表
void DoubleList_destroy(DoubleList * dl)
{
	for (struct Node * p = dl->head->nxt; p != dl->tail; p = p->nxt)
		DoubleList_deleteNode(p->pre);

	free(dl->tail->pre);
	free(dl->tail);
}

// 打印双向链表
void DoubleList_print(DoubleList * dl)
{
	printf("\n\t——————————————————————————当前链表——————————————————————————\n");
	for (struct Node * p = dl->head->nxt; p != dl->tail; p = p->nxt)
	{
		printf("\t%s \t", p->data.key.domain);
		if (p->data.key.isIpv4 == true) {
			for (int i = 0; i < IP4SIZE; i++)
				printf("%d%s", p->data.ip[i], i == IP4SIZE-1 ? "\n" : ".");
		} else {
			for (int i = 0; i < IP6SIZE; i++)
				printf("%02x%s", p->data.ip[i], i == IP6SIZE -1?"\n":( i % 2 == 1 ? ":" : ""));
		}


		printf("");
	}
	printf("\t———————————————————————————————————————-————————————————————\n\n");
}

// 拷贝数据
void DoubleList_copyData(bpDATATYPE * d, bpDATATYPE * s)
{
	d->key.isIpv4 = s->key.isIpv4;
	d->key.domain = deepCopyStr(s->key.domain);
	d->deadTime = s->deadTime;
	strncpy(d->ip, s->ip, s->key.isIpv4 ? IP4SIZE : IP6SIZE);
}

// 从双向链表中移除某节点
bpDATATYPE DoubleList_remove(DoubleList * dl, struct Node * p)
{
	p->pre->nxt = p->nxt;
	p->nxt->pre = p->pre;
	bpDATATYPE data;
	DoubleList_copyData(&data, &(p->data));
	DoubleList_deleteNode(p);
	return data;
}

// 找下一个过时的项
void DoubleList_findNextTTLOut(DoubleList * dl, struct Node ** p)
{
	for (struct Node * pp = *p; pp != dl->tail && time(NULL) < pp->data.deadTime; pp = pp->nxt)
		*p = pp;
}

// 销毁节点
void DoubleList_deleteNode(struct Node * p)
{
	free(p->data.key.domain);
	free(p);
}