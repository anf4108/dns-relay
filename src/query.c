#include "query.h"
#include "config.h"

int lookup_int_text(char *dn, char *ip) {
    if (BufferPool_find(bp, dn, 1, ip)) {
        return 1; // 缓存命中
    }
    return 0;
}