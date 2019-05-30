CFLAGS := -g -Wall -Wextra -Wpedantic
LFLAGS := -g -Wall -Wextra -Wpedantic

bfasm: main.o tokenizer.o mnemonic.o
	gcc $^ -o $@
