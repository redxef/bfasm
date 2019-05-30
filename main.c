#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "mnemonic.h"
#include "tokenizer.h"

#define MNEMONIC_MAXKNOWN       1024
#define MAXTOKENS               4096


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
