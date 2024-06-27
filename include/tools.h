#ifndef TOOLS_H
#define TOOLS_H

typedef enum {
	EMPTY,
	DELETED,
	VALID
} CellState;

// 素数判别
extern bool prime(int num);

// 找到最近的4k+3的素数（为了平方探测）
extern int closestPrime(int capacity);

extern char * deepCopyStr(const char * p);

#endif