#include <stdint.h>
#include <stdio.h>
#include "mnemonic.h"

#ifndef TOKENIZER_H
#define TOKENIZER_H

#define TOKENIZER_MAXGENERATORS 64
#define TOKENCONS_MAXCONSUMERS  64

#define TOKEN_NONE              0x00
#define TOKEN_SPACE             0x10
#define TOKEN_NEWLINE           0x11
#define TOKEN_NUMBER            0x20
#define TOKEN_ALPHA             0x40
#define TOKEN_OPERATOR          0x50

struct token {
        int type;
        union {
                void *ptr;
                uint64_t valu;
                int64_t vali;
                long double valf;
        };
};

typedef const char *(*tokengen)(const char *, struct token *, void *);

struct tokenizer {
        struct {
                tokengen tgen;
                void *extargs;
        } tgens[TOKENIZER_MAXGENERATORS];
};

struct token_consumer;

typedef struct token *(*gettoken)(void *);
typedef size_t (*tokencons)(struct token_consumer *, void *);

struct token_consumer {
        struct tokenizer *t;
        struct {
                tokencons tcons;
                void *extargs_tcons;
        } tconss[TOKENCONS_MAXCONSUMERS];
        gettoken ct;
        gettoken nt;
        gettoken pt;
        void *args_gettoken;

};

void token_fprint(FILE *f, struct token *t);

void tokenizer_init(struct tokenizer *t);
int tokenizer_add_tokengenerator(struct tokenizer *t, tokengen tgen, void *extra_args);
int tokenizer_add_default_tokengens(struct tokenizer *t);
const char *tokenizer_get_next_token(struct tokenizer *t, const char *str, struct token *tkn);

void token_consumer_init(struct token_consumer *tc, struct tokenizer *t, gettoken *gtokens, void *args_gtoken);
int token_consumer_add_consumer(struct token_consumer *tc, tokencons tcons, void *extargs);
size_t token_consumer_consume(struct token_consumer *tc);

#endif
