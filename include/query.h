#ifndef QUERY_H
#define QUERY_H

#include <stdlib.h>
#include <string.h>
#include <bufferpool.h>

int lookup_int_text(char *dn, char *ip);
int lookup_int_text_ipv6(char *dn, char *ip);

#endif