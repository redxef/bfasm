CFLAGS := -g -Wall -Wextra -Wpedantic
LFLAGS := -g -Wall -Wextra -Wpedantic

bfasm: main.o tokenizer.o mnemonic.o util.o
	gcc $^ -o $@

clean:
	$(RM) $(wildcard *.o)
	$(RM) bfasm

.PHONY: clean
