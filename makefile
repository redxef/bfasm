CFLAGS := -g -Wall -Wextra -Wpedantic
LFLAGS := -g -Wall -Wextra -Wpedantic

bfasm: main.o
	gcc main.o -o $@
