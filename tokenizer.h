#include <stdint.h>
#include <stdio.h>
#include "mnemonic.h"

#ifndef TOKENIZER_H
#define TOKENIZER_H

#define TOKENIZER_BUFFLEN       32
#define MNEMONIC_MAXTOKENCNT    1024

#define TOKEN_NONE              0x00
#define TOKEN_SPACE             0x10
#define TOKEN_NEWLINE           0x11
#define TOKEN_NUMBER            0x20
#define TOKEN_VARIABLE          0x30
#define TOKEN_MNEMONIC          0x40
#define TOKEN_OPERATOR          0x50

struct token {
        int type;
        union {
                void *ptr;
                int val;
        };
};

void token_fprint(FILE *f, struct token *t);

const char *token_space(const char *c, struct token *t);
const char *token_newline(const char *c, struct token *t);
const char *token_variable(const char *c, struct token *t);
const char *token_number(const char *c, struct token *t);
const char *token_operator(const char *c, struct token *t);
const char *token_mnemonic(const char *c, struct token *t, ssize_t mslen, struct mnemonic *ms);

ssize_t token_consume(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen);
ssize_t token_consume_operator(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen);
ssize_t token_consume_mnemonic(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen);
ssize_t token_consume_number(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen);
ssize_t token_consume_variable(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen);
ssize_t token_consume_newline(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen);
ssize_t token_consume_space(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen);

void token_tokenize(FILE *f, struct token *tokens, struct mnemonic *ms, ssize_t mslen);

#endif
