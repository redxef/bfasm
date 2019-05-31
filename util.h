#include <stdlib.h>

#ifndef UTIL_H
#define UTIL_H

void strsl(char *c, int clen, int cnt);
void strsr(char *c, int clen, int cnt);
void memsl(void *arr, size_t objsize, size_t len, size_t cnt, void *fill);
void memsr(void *arr, size_t objsize, size_t len, size_t cnt, void *fill);

#endif
