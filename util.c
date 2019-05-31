#include <stdlib.h>
#include <string.h>
#include "util.h"

void strsl(char *c, int clen, int cnt) {
        int i;
        for (i = 0; i < clen - cnt; i++) {
                c[i] = c[i+cnt];
        }
        for (i = clen - cnt; i < clen; i++) {
                c[i] = 0;
        }
}

void strsr(char *c, int clen, int cnt) {
        int i;
        for (i = clen - 1; i >= cnt; i--) {
                c[i] = c[i-cnt];
        }
        for (i = 0; i < cnt; i++) {
                c[i] = 0;
        }
}

void memsl(void *arr, size_t objsize, size_t len, size_t cnt, void *fill) {
        char *a = arr;
        size_t i;
        char fill_[objsize];
        if (fill == NULL) {
                memset(fill_, 0, objsize);
                fill = fill_;
        }
        for (i = 0; i < len - cnt; i++) {
                memcpy(a + i * objsize, a + (i + cnt) * objsize, objsize);
        }
        for (i = len - cnt; i < len; i++) {
                memcpy(a + i * objsize, fill, objsize);
        }
}

void memsr(void *arr, size_t objsize, size_t len, size_t cnt, void *fill) {
        char *a = arr;
        size_t i;
        char fill_[objsize];
        if (fill == NULL) {
                memset(fill_, 0, objsize);
                fill = fill_;
        }
        for (i = len - 1; i >= cnt; i--) {
                memcpy(a + i * objsize, a + (i - cnt) * objsize, objsize);
        }
        for (i = 0; i < cnt; i++) {
                memcpy(a + i * objsize, fill, objsize);
        }
}
