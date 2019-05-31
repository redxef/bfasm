#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include "mnemonic.h"
#include "tokenizer.h"
#include "util.h"

#define MNEMONIC_MAXKNOWN       256
#define MAXTOKENS               8192

#define TOKEN_MNEMONIC          0x1010
#define TOKEN_VARIABLE          0x1020

struct extargs_tc {
        FILE *fout;
        struct mnemonic *mlist;
        ssize_t mlistlen;
};

struct next_token_args {
        struct token *tlist;
        size_t tlistlen;
        size_t i;
};

#define TMP_ARGS        struct token_consumer *tc, void *args

size_t tokcons_consume_none(TMP_ARGS) {
        (void) tc;
        (void) args;
        struct token *t = tc->ct(tc->args_gettoken);
        if (t->type != TOKEN_NONE)
                return 0;
        return 1;
}

size_t tokcons_consume_space(TMP_ARGS) {
        (void) tc;
        (void) args;
        struct token *t = tc->ct(tc->args_gettoken);
        if (t->type != TOKEN_SPACE)
                return 0;
        return 1;
}

size_t tokcons_consume_newline(TMP_ARGS) {
        (void) tc;
        struct extargs_tc *argstc = args;
        struct token *t = tc->ct(tc->args_gettoken);

        if (t->type != TOKEN_NEWLINE)
                return 0;
        fputc('\n', argstc->fout);
        return 1;
}

size_t tokcons_consume_uinteger(TMP_ARGS) {
        (void) args;
        int i;
        size_t cons_cnt = 0;
        struct token *t = tc->ct(tc->args_gettoken);

        if (t->type != TOKEN_NUMBER)
                return 0;

        tc->nt(tc->args_gettoken);
        for (i = 0; i < t->vali; i++) {
                cons_cnt = token_consumer_consume(tc);
        }
        tc->pt(tc->args_gettoken);
        return 1 + cons_cnt;
}

size_t tokcons_consume_operator(TMP_ARGS) {
        (void) tc;
        struct extargs_tc *argstc = args;
        struct token *t = tc->ct(tc->args_gettoken);

        if (t->type != TOKEN_OPERATOR)
                return 0;
        fputc(t->valu, argstc->fout);
        return 1;
}

size_t tokcons_consume_mnemonic(TMP_ARGS) {
        struct token *t = tc->ct(tc->args_gettoken);
        struct token *t0;
        struct extargs_tc *argstc = args;
        struct next_token_args *nta_old;
        struct next_token_args nta_tmp;
        struct mnemonic key;
        const char *c;
        int argcnt;
        struct token tlist[MAXTOKENS>>2];
        size_t i;
        int next_is_var;

        if (t->type == TOKEN_MNEMONIC)
                goto SKIP_CHECK;
        if (t->type != TOKEN_ALPHA)
                return 0;

        t0 = tc->nt(tc->args_gettoken);
        if (t0->type == TOKEN_OPERATOR && t0->valu == '(')
                return 0;
        tc->pt(tc->args_gettoken);
        t->type = TOKEN_MNEMONIC;
        memset(&key, 0, sizeof(key));
        key.name = t->ptr;
        t->ptr = bsearch(&key, argstc->mlist, argstc->mlistlen, sizeof(*argstc->mlist), mnemonic_cmp_name);
        if (t->ptr == NULL) {
                fprintf(stderr, "Error: No such known mnemonic\n");
                free(key.name);
                abort();
        }
        free(key.name);

SKIP_CHECK:
        c = ((struct mnemonic *)t->ptr)->val;
        argcnt = ((struct mnemonic *)t->ptr)->argcnt;

        nta_old = tc->args_gettoken;
        i = 0;
        next_is_var = 0;
        memset(&tlist, 0, sizeof(tlist));
        while (*c) {
                c = tokenizer_get_next_token(tc->t, c, tlist + i);
                if (next_is_var) {
                        tlist[i].type = TOKEN_NUMBER;
                        tlist[i].valu = nta_old->tlist[nta_old->i + tlist[i].valu].valu;
                        next_is_var = 0;
                } else if (tlist[i].type == TOKEN_OPERATOR && tlist[i].valu == '$') {
                        memset(tlist + i, 0, sizeof(*tlist));
                        next_is_var = 1;
                        i--;
                }
                i++;
        }

        nta_tmp.tlist = tlist;
        nta_tmp.tlistlen = 100;
        nta_tmp.i = 0;
        tc->args_gettoken = &nta_tmp;
        i = 0;
        while (tc->ct(tc->args_gettoken) != NULL) {
                i = nta_tmp.i;
                nta_tmp.i += token_consumer_consume(tc);
                if (nta_tmp.i == i) {
                        fprintf(stderr, "Error: Unknown token ```\n");
                        token_fprint(stderr, tc->ct(tc->args_gettoken));
                        fprintf(stderr, "'''\n");
                        if (tc->ct(tc->args_gettoken)->type == TOKEN_MNEMONIC) {
                                mnemonic_fprint(stderr, tc->ct(tc->args_gettoken)->ptr);
                        }
                        break;
                }
        }
        tc->args_gettoken = nta_old;


        return 1 + argcnt;
}

struct token *get_current_token(void *ptr) {
        struct next_token_args *nargs = ptr;
        if (nargs->i >= nargs->tlistlen)
                return NULL;
        return &nargs->tlist[nargs->i];
}

struct token *get_next_token(void *ptr) {
        struct next_token_args *nargs = ptr;
        nargs->i++;
        return get_current_token(ptr);
}

struct token *get_prev_token(void *ptr) {
        struct next_token_args *nargs = ptr;
        nargs->i--;
        return get_current_token(ptr);
}

void setup_tokenizer(struct tokenizer *t) {
        tokenizer_init(t);
        tokenizer_add_default_tokengens(t);
}

void setup_token_consumer(
                struct token_consumer *tc,
                struct tokenizer *t,
                struct next_token_args *nta,
                struct extargs_tc *extatc,
                struct token *tlist, size_t tlistlen,
                struct mnemonic *mlist, size_t mlistlen,
                FILE *outfile) {
        gettoken gtokens[3];

        gtokens[0] = get_current_token;
        gtokens[1] = get_next_token;
        gtokens[2] = get_prev_token;

        nta->tlist = tlist;
        nta->tlistlen = tlistlen;
        nta->i = 0;

        extatc->fout = outfile;
        extatc->mlist = mlist;
        extatc->mlistlen = mlistlen;

        token_consumer_init(tc, t, gtokens, nta);
        token_consumer_add_consumer(tc, tokcons_consume_none, extatc);
        token_consumer_add_consumer(tc, tokcons_consume_space, extatc);
        token_consumer_add_consumer(tc, tokcons_consume_newline, extatc);
        token_consumer_add_consumer(tc, tokcons_consume_uinteger, extatc);
        token_consumer_add_consumer(tc, tokcons_consume_operator, extatc);
        token_consumer_add_consumer(tc, tokcons_consume_mnemonic, extatc);
}

int main(int argc, char **argv) {
        (void) argc;
        (void) argv;
        struct mnemonic mlist[MNEMONIC_MAXKNOWN];
        struct token tlist[MAXTOKENS];
        struct tokenizer t;
        struct token_consumer tc;
        struct next_token_args nta;
        struct extargs_tc extatc;
        FILE *mnemonic_in;
        FILE *infile, *outfile;
        size_t i;
        size_t filesize;
        ssize_t ii;
        char *src;
        char *c;

        infile = stdin;
        outfile = stdout;

        const char *shoptlist = "hl:L:c:";
        struct option optlist[] = {
                {
                        "help",
                        no_argument,
                        NULL,
                        'h'
                },
                {
                        "load-mnemonic",
                        required_argument,
                        NULL,
                        'l'
                },
                {
                        "load-macro",
                        required_argument,
                        NULL,
                        'L'
                },
                {
                        "compile",
                        required_argument,
                        NULL,
                        'c'
                },
                {
                        NULL,
                        0,
                        NULL,
                        0
                },
        };
        char optc;
        int optidx = 0;

        memset(mlist, 0, sizeof(mlist));
        memset(tlist, 0, sizeof(tlist));

        while (1) {
                optc = getopt_long(argc, argv, shoptlist, optlist, &optidx);

                if (optc == -1)
                        break;

                switch (optc) {
                        case 'h':
                                printf("HELP!\n");
                                return 0;
                        case 'l':
                                mnemonic_in = fopen(optarg, "r");
                                mnemonic_load_file(mnemonic_in, mlist, MNEMONIC_MAXKNOWN);
                                fclose(mnemonic_in);
                                break;
                        case 'L':
                                printf("Not yet implemented!\n");
                                break;
                        case 'c':
                                infile = fmemopen(optarg, strlen(optarg) + 1, "r");
                                break;
                        case 'o':
                                outfile = fopen(optarg, "w");
                                break;
                        default:
                                break;
                }
        }

        qsort(mlist, MNEMONIC_MAXKNOWN, sizeof(*mlist), mnemonic_cmp_name);

        setup_tokenizer(&t);
        setup_token_consumer(&tc, &t, &nta, &extatc, tlist, MAXTOKENS, mlist, MNEMONIC_MAXKNOWN, outfile);

        fseek(infile, 0l, SEEK_END);
        filesize = ftell(infile) + 2;
        rewind(infile);

        src = malloc(filesize);
        c = src;
        while ((ii = fgetc(infile)) != EOF)  {
                *c++ = ii;
        }
        if (*(c-1) != '\n') {
                *(c-1) = '\n';
        }
        *c = '\0';
        c = src;
        i = 0;
        while (*c) {
                c = (char *) tokenizer_get_next_token(&t, c, &tlist[i++]);
        }
        free(src);
        while (tc.ct(tc.args_gettoken) != NULL) {
                i = nta.i;
                nta.i += token_consumer_consume(&tc);
                if (i == nta.i) {
                        fprintf(stderr, "Error: Unknown token ```\n");
                        token_fprint(stderr, tc.ct(tc.args_gettoken));
                        if (tc.ct(tc.args_gettoken)->type == TOKEN_MNEMONIC) {
                                mnemonic_fprint(stderr, tc.ct(tc.args_gettoken)->ptr);
                        }
                        fprintf(stderr, "'''\n");
                        break;
                }
        }
        fclose(infile);
        fclose(outfile);
}
