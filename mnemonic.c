#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "mnemonic.h"


void mnemonic_create(struct mnemonic *m, char *n, char *v, int argc) {
        m->name = n;
        m->val = v;
        m->argcnt = argc;
}

int mnemonic_cmp_name(const void *ma, const void *mb) {
        const struct mnemonic *m0 = ma;
        const struct mnemonic *m1 = mb;

        if (m0->name == NULL && m1->name == NULL)
                return 0;
        else if (m0->name == NULL && m1->name != NULL)
                return 1;
        else if (m0->name != NULL && m1->name == NULL)
                return -1;

        return strcmp(m0->name, m1->name);
}

void mnemonic_load_from_str_old(struct mnemonic *m, const char *str) {
        ssize_t slen = strlen(str);
        int field = 0;
        int numargs = 0;
        int i;
        char *s = malloc(slen+1);
        char *start;
        char *end;
        char *strings[3];
        int vars[9] = {0};

        assert(s);

        strcpy(s, str);
        start = s;

        do {
                while (*start && isspace(*start))
                        start++;
                end = start;
                while (*end && !isspace(*end))
                        end++;
                *end = '\0';
                strings[field++] = start;
                start = end+1;
        } while (((end - s) < slen) && (field < (int) (sizeof(strings)/sizeof(strings[0]))));

        m->name = malloc(strlen(strings[0])+1);
        m->val = malloc(strlen(strings[1])+1);
        assert(m->name);
        assert(m->val);
        strcpy(m->name, strings[0]);
        strcpy(m->val, strings[1]);

        start = strings[1];
        end = strings[1] + strlen(strings[1]+1);
        while (start < end) {
                if (*start == '$') {
                        if (isdigit(start[1])) {
                                vars[start[1]-'0']++;
                        }
                }
                start++;
        }
        for (i = 0; i < 9; i++) {
                numargs += (vars[i] > 0);
        }
        m->argcnt = numargs;
        free(s);
}

void mnemonic_load_from_str(struct mnemonic *m, const char *str) {
        char *str_cpy;
        char *start_mname;
        char *start_mdef;
        char *c;
        int flag = 0, i;
        int mnemonic_vars[MNEMONIC_MAXVARS];

        str_cpy = malloc(strlen(str) + 1);
        strcpy(str_cpy, str);
        start_mname = str_cpy;

        while (isspace(*start_mname))
                start_mname++;
        start_mdef = start_mname;
        while (!isspace(*start_mdef))
                start_mdef++;
        *start_mdef = '\0';
        start_mdef++;
        while (isspace(*start_mdef))
                start_mdef++;

        m->name = start_mname;
        m->val = start_mdef;

        memset(mnemonic_vars, 0, sizeof(mnemonic_vars));
        c = m->val;
        while (*c) {
                if (flag) {
                        mnemonic_vars[*c - '0']++;
                        flag = 0;
                }
                if (*c == '$') {
                        flag = 1;
                }
                c++;
        }
        for (i = 0; i < MNEMONIC_MAXVARS; i++) {
                m->argcnt += mnemonic_vars[i] > 0;
        }
}

void mnemonic_fprint(FILE *f, struct mnemonic *m) {
        fprintf(f, "struct mnemonic {\n");
        fprintf(f, "    name   = %s\n", m->name);
        fprintf(f, "    val    = %s\n", m->val);
        fprintf(f, "    argcnt = %d\n", m->argcnt);
        fprintf(f, "}\n");
}

void mnemonic_load_file(FILE *f, struct mnemonic *ms, ssize_t mslen) {
        char buff[1024];
        char *c;
        int i = 0;

        while (fgets(buff, 1024, f) && i < mslen) {
                c = strchr(buff, '\n');
                if (c != NULL)
                        *c = '\0';
                mnemonic_load_from_str(&ms[i++], buff);
        }

}
