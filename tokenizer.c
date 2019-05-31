#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "mnemonic.h"
#include "tokenizer.h"


const char *token_getnext_space(const char *c, struct token *t, void * ea) {
        (void) ea;
        while (isspace(*c) && *c != '\n') {
                t->vali++;
                c++;
        }
        if (t->vali)
                t->type = TOKEN_SPACE;
        return c;
}

const char *token_getnext_newline(const char *c, struct token *t, void *ea) {
        (void) ea;
        if (*c != '\n')
                return c;
        t->type = TOKEN_NEWLINE;
        t->vali = 1;
        return c+1;
}

const char *token_getnext_uinteger(const char *c, struct token *t, void *ea) {
        (void) ea;
        const char *c0 = c;
        int base = 10;
        if (c0[0] == '0') {
                switch (c0[1]) {
                        case 'x':
                                base = 16;
                                c0 += 2;
                                break;
                        case 'b':
                                base = 2;
                                c0 += 2;
                                break;
                        default:
                                base = 8;
                                c0 += 1;
                                break;
                }
        }
        while (base > 10 ? isxdigit(*c0) : isdigit(*c0)) {
                t->valu *= base;
                if (*c0 >= '0' && *c <= '9')
                        t->valu += (*c0) - '0';
                if (*c0 >= 'a' && *c <= 'f')
                        t->valu += (*c0) - 'a';
                if (*c0 >= 'A' && *c <= 'F')
                        t->valu += (*c0) - 'A';
                c0++;
        }
        if (c0 - c)
                t->type = TOKEN_NUMBER;
        return c0;
}

const char *token_getnext_operator(const char *c, struct token *t, void *ea) {
        (void) ea;
        if (ispunct(*c)) {
                t->type = TOKEN_OPERATOR;
                t->vali = *c;
                return c + 1;
        }
        return c;
}

const char *token_getnext_alpha(const char *c, struct token *t, void *ea) {
        (void) ea;
        const char *c0 = c;
        size_t len = 0;
        while (isalpha(*c0))
                c0++;
        len = c0 - c;
        if (len == 0)
                return c;
        t->type = TOKEN_ALPHA;
        t->ptr = malloc(len + 1);
        memcpy(t->ptr, c, len);
        ((char *)t->ptr)[len] = '\0';
        return c0;
}

const char *tokenizer_get_next_token(struct tokenizer *t, const char *str, struct token *tkn) {
        int i;
        const char *curc;
        for (i = 0; i < TOKENIZER_MAXGENERATORS && t->tgens[i].tgen != NULL; i++) {
                curc = t->tgens[i].tgen(str, tkn, t->tgens[i].extargs);
                if (curc != str)
                        return curc;
        }
        return str;
}

void tokenizer_init(struct tokenizer *t) {
        memset(t, 0, sizeof(struct tokenizer));
}

int tokenizer_add_tokengenerator(struct tokenizer *t, tokengen tgen, void *extra_args) {
        int i;

        for (i = 0; i < TOKENIZER_MAXGENERATORS; i++) {
                if (t->tgens[i].tgen == NULL) {
                        t->tgens[i].tgen = tgen;
                        t->tgens[i].extargs = extra_args;
                        return 1;
                }
        }
        return 0;
}

int tokenizer_add_default_tokengens(struct tokenizer *t) {
        int res = 0;
        res |= tokenizer_add_tokengenerator(t, token_getnext_space, NULL);
        res |= tokenizer_add_tokengenerator(t, token_getnext_newline, NULL);
        res |= tokenizer_add_tokengenerator(t, token_getnext_uinteger, NULL);
        res |= tokenizer_add_tokengenerator(t, token_getnext_operator, NULL);
        res |= tokenizer_add_tokengenerator(t, token_getnext_alpha, NULL);
        return res;
}

void token_fprint(FILE *f, struct token *t) {
        fprintf(f, "struct token {\n");
        fprintf(f, "    type = 0x%02x\n", t->type);
        fprintf(f, "    ptr  = %p\n", t->ptr);
        if (t->type == TOKEN_ALPHA)
                fprintf(f, "    str  = %s\n", (char *) t->ptr);
        fprintf(f, "    vali = %ld\n", t->vali);
        fprintf(f, "}\n");
}

void token_consumer_init(struct token_consumer *tc, struct tokenizer *t, gettoken *gtokens, void *args_gtoken) {
        memset(tc, 0, sizeof(struct token_consumer));
        tc->t = t;
        tc->ct = gtokens[0];
        tc->nt = gtokens[1];
        tc->pt = gtokens[2];
        tc->args_gettoken = args_gtoken;
}

int token_consumer_add_consumer(struct token_consumer *tc, tokencons tcons, void *extargs) {
        int i;

        for (i = 0; i < TOKENCONS_MAXCONSUMERS; i++) {
                if (tc->tconss[i].tcons == NULL) {
                        tc->tconss[i].tcons = tcons;
                        tc->tconss[i].extargs_tcons = extargs;
                        return 1;
                }
        }
        return 0;
}

size_t token_consumer_consume(struct token_consumer *tc) {
        int i;
        int consumed;

        for (i = 0; i < TOKENCONS_MAXCONSUMERS && tc->tconss[i].tcons != NULL; i++) {
                consumed = tc->tconss[i].tcons(tc, tc->tconss[i].extargs_tcons);
                if (consumed)
                        return consumed;
        }
        return 0;

}
