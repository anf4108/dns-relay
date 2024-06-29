#include "query.h"
#include "config.h"

int lookup_in_cache(char *dn, char *ip) {
    if (BufferPool_find(bp, dn, 1, ip)) {
        return 1; // 缓存命中
    }
    return 0;
}

int lookup_in_cache_ipv6(char *dn, char *ip) {
    if (BufferPool_find(bp, dn, 1, ip)) {
        return 1; // 缓存命中
    }
    return 0;
}