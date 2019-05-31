#include <stdint.h>
#include <stdio.h>

#ifndef MNEMONIC_H
#define MNEMONIC_H

#define MNEMONIC_MAXLEN         8
#define MNEMONIC_MAXVARS        8

struct mnemonic {
        char *name;
        char *val;
        int argcnt;
};

void mnemonic_create(struct mnemonic *m, char *n, char *v, int argc);
int mnemonic_cmp_name(const void *ma, const void *mb);
void mnemonic_load_from_str(struct mnemonic *m, const char *str);
void mnemonic_fprint(FILE *f, struct mnemonic *m);
void mnemonic_load_file(FILE *f, struct mnemonic *ms, ssize_t mslen);

#endif
