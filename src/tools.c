#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// 素数判别
bool prime(int num) {
	if (num <= 1) return false;
	if (num <= 3) return true;
	if (num % 2 == 0 || num % 3 == 0) return false;

	for (int i = 5; i * i <= num; i += 6) {
		if (num % i == 0 || num % (i + 2) == 0) {
			return false;
		}
	}
	return true;
}

// 找到最近的4k+3的素数（为了平方探测）
int closestPrime(int capacity) {
	int candidate = capacity / 4 * 4 + 3;
	while (!prime(candidate)) {
		candidate -= 4;
	}
	return candidate;
}

char * deepCopyStr(char * p)
{
	char * s = (char *)malloc(sizeof(char) * strlen(p));
	if (s == NULL)
	{
		printf("[失败] malloc char");
		exit(-1);
	}
	strcpy(s, p);
	return s;
}