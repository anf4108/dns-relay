#ifndef QUERY_H
#define QUERY_H

#include <stdlib.h>
#include <string.h>
#include <bufferpool.h>

int lookup_in_cache(char *dn, char *ip);
int lookup_in_cache_ipv6(char *dn, char *ip);

#endif