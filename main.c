#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define MNEMONIC_MAXLEN         8
#define TOKENIZER_BUFFLEN       32
#define MNEMONIC_MAXKNOWN       1024
#define MNEMONIC_MAXTOKENCNT    1024
#define MAXTOKENS               4096

#define TOKEN_NONE              0x00
#define TOKEN_SPACE             0x10
#define TOKEN_NEWLINE           0x11
#define TOKEN_NUMBER            0x20
#define TOKEN_VARIABLE          0x30
#define TOKEN_MNEMONIC          0x40
#define TOKEN_OPERATOR          0x50

struct mnemonic {
        char *name;
        char *val;
        int argcnt;
};

struct mnemonic_par {
        struct mnemonic m;
        char **argv;
};

struct token {
        int type;
        union {
                void *ptr;
                int val;
        };
};

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

void mnemonic_load_from_str(struct mnemonic *m, const char *str) {
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
        } while (((end - s) < slen) && (field < (int) sizeof(strings)/sizeof(strings[0])));

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

void mnemonic_fprint(FILE *f, struct mnemonic *m) {
        fprintf(f, "struct mnemonic {\n");
        fprintf(f, "    name   = %s\n", m->name);
        fprintf(f, "    val    = %s\n", m->val);
        fprintf(f, "    argcnt = %d\n", m->argcnt);
        fprintf(f, "}\n");
}

const char *token_space(const char *c, struct token *t) {
        while (isspace(*c) && *c != '\n') {
                t->val++;
                c++;
        }
        if (t->val)
                t->type = TOKEN_SPACE;
        t->type = TOKEN_NONE;
        t->val = 0;
        return c;
}

const char *token_newline(const char *c, struct token *t) {
        if (*c != '\n')
                return c;
        t->type = TOKEN_NEWLINE;
        t->val = 1;
        return c+1;
}

const char *token_variable(const char *c, struct token *t) {
        if (*c != '$')
                return c;
        c++;
        t->type = TOKEN_VARIABLE;
        while (isdigit(*c)) {
                t->val *= 10;
                t->val += (*c) - '0';
                c++;
        }
        return c;
}

const char *token_number(const char *c, struct token *t) {
        const char *c0 = c;
        while (isdigit(*c0)) {
                t->val *= 10;
                t->val += (*c0) - '0';
                c0++;
        }
        if (c0 - c)
                t->type = TOKEN_NUMBER;
        return c0;
}

const char *token_mnemonic(const char *c, struct token *t, ssize_t mslen, struct mnemonic *ms) {
        const char *c0 = c;
        char mnembuff[MNEMONIC_MAXLEN + 1];
        struct mnemonic key, *res;
        key.name = mnembuff;

        memset(mnembuff, 0, MNEMONIC_MAXLEN + 1);

        while (isalpha(*c0)) {
                key.name[c0-c] = *c0;
                c0++;
        }
        key.name[c0-c] = '\0';
        if ((c0 - c) == 0)
                return c0;

        res = bsearch(&key, ms, mslen, sizeof(*ms), mnemonic_cmp_name);
        if (res == NULL) {
                fprintf(stderr, "Error: no such known mnemonic: `%s'\n", key.name);
                return c0;
        }
        t->type = TOKEN_MNEMONIC;
        t->ptr = res;
        return c0;
}

const char *token_operator(const char *c, struct token *t) {
        if (strchr("[]+-<>.,", *c)) {
                t->type = TOKEN_OPERATOR;
                t->val = *c;
                return c+1;
        }
        return c;
}

void token_fprint(FILE *f, struct token *t) {
        fprintf(f, "struct token {\n");
        fprintf(f, "    type = 0x%02x\n", t->type);
        fprintf(f, "    ptr  = %p\n", t->ptr);
        fprintf(f, "    val  = %d\n", t->val);
        fprintf(f, "}\n");
}

void strsl(char *c, int clen, int cnt) {
        int i;
        for (i = 0; i < clen - cnt; i++) {
                c[i] = c[i+cnt];
        }
        for (i = clen - cnt; i < clen; i++) {
                c[i] = 0;
        }
}

void token_tokenize(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen) {
        int c;
        char *newc;
        char buff[TOKENIZER_BUFFLEN];
        int buffcntr = 0;
        int tokencntr = 0;

        memset(buff, 0, TOKENIZER_BUFFLEN);

        while ((c = fgetc(f)) != EOF || strlen(buff)) {
                if (c != EOF)
                        buff[buffcntr++] = c;
                if (c != EOF && strlen(buff) < MNEMONIC_MAXLEN)
                        continue;
                newc = (char *) token_space(buff, &tokens[tokencntr]);
                if (newc - buff)
                        goto WORK;
                newc = (char *) token_newline(buff, &tokens[tokencntr]);
                if (newc - buff)
                        goto WORK;
                newc = (char *) token_variable(buff, &tokens[tokencntr]);
                if (newc - buff)
                        goto WORK;
                newc = (char *) token_number(buff, &tokens[tokencntr]);
                if (newc - buff)
                        goto WORK;
                newc = (char *) token_mnemonic(buff, &tokens[tokencntr], mslen, ms);
                if (newc - buff)
                        goto WORK;
                newc = (char *) token_operator(buff, &tokens[tokencntr]);
                if (newc - buff)
                        goto WORK;

                if (c == EOF) {
                        buffcntr = 0;
                        memset(buff, 0, TOKENIZER_BUFFLEN);
                }
                continue;
WORK:
                if (tokens[tokencntr].type != TOKEN_NONE)
                        tokencntr++;
                strsl(buff, TOKENIZER_BUFFLEN, newc - buff);
                buffcntr -= newc-buff;
        }
}

// TODO: syntax rules...

ssize_t token_consume(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen);

ssize_t token_consume_space(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen) {
        (void) f;
        (void) tokens;
        (void) ms;
        (void) mslen;
        return 1;
}

ssize_t token_consume_newline(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen) {
        (void) tokens;
        (void) ms;
        (void) mslen;
        fputc('\n', f);
        return 1;
}

ssize_t token_consume_variable(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen) {
        (void) f;
        (void) tokens;
        (void) ms;
        (void) mslen;
        fprintf(stderr, "Error: Not yet implemented, skipping...\n");
        return 1;
}

ssize_t token_consume_number(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen) {
        int i;
        for (i = 0; i < tokens->val; i++) {
                token_consume(f, tokens+1, ms, mslen);
        }
        return 2;
}

ssize_t token_consume_mnemonic(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen) {
        struct mnemonic *m = tokens->ptr;
        struct token tokens0[MNEMONIC_MAXTOKENCNT];
        int i;
        FILE *f0;
        memset(tokens0, 0, sizeof(tokens0));

        f0 = fmemopen(m->val, strlen(m->val), "r");
        token_tokenize(f0, tokens0, ms, mslen);
        fclose(f0);

        for (i = 0; i < MNEMONIC_MAXTOKENCNT; i++) {
                if (tokens0[i].type == TOKEN_VARIABLE) {
                        tokens0[i].type = TOKEN_NUMBER;
                        tokens0[i].val = tokens[tokens0[i].val].val;
                }
        }

        for (i = 0; i < MNEMONIC_MAXTOKENCNT; i += token_consume(f, &tokens0[i], ms, mslen));
        return 1+((struct mnemonic *)tokens->ptr)->argcnt;
}

ssize_t token_consume_operator(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen) {
        (void) ms;
        (void) mslen;
        fputc(tokens->val, f);
        return 1;
}

ssize_t token_consume(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen) {
        switch (tokens->type) {
                case TOKEN_NONE:
                default:
                        return 1;
                case TOKEN_SPACE:
                        return token_consume_space(f, tokens, ms, mslen);
                case TOKEN_NEWLINE:
                        return token_consume_newline(f, tokens, ms, mslen);
                case TOKEN_VARIABLE:
                        return token_consume_variable(f, tokens, ms, mslen);
                case TOKEN_NUMBER:
                        return token_consume_number(f, tokens, ms, mslen);
                case TOKEN_MNEMONIC:
                        return token_consume_mnemonic(f, tokens, ms, mslen);
                case TOKEN_OPERATOR:
                        return token_consume_operator(f, tokens, ms, mslen);
        }
}

void mnemonic_load_file(FILE *f, struct mnemonic *ms, ssize_t mslen) {
        char buff[1024];
        int i = 0;

        while (fgets(buff, 1024, f) && i < mslen)
                mnemonic_load_from_str(&ms[i++], buff);

}

int main(int argc, char **argv) {
        (void) argc;
        (void) argv;
        FILE *mnemonic_in = fopen("./mnemonics.txt", "r");
        struct mnemonic ms[MNEMONIC_MAXKNOWN];

        struct token tokens[MAXTOKENS];
        struct token *flow;

        memset(ms, 0, sizeof(ms));
        memset(tokens, 0, sizeof(tokens));

        mnemonic_load_file(mnemonic_in, ms, MNEMONIC_MAXKNOWN);
        qsort(ms, MNEMONIC_MAXKNOWN, sizeof(*ms), mnemonic_cmp_name);

        token_tokenize(stdin, tokens, ms, MNEMONIC_MAXKNOWN);

        flow = tokens;
        while (flow->type != TOKEN_NONE)
                flow += token_consume(stdout, flow, ms, MNEMONIC_MAXKNOWN);

        return 0;
}
