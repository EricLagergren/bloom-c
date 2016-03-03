CC=gcc
CFLAGS=-Wall -Werror -Wextra -pedantic -O3
SRC=main.c bloom.c SipHash/siphash24.c
BIN=main

all: $(BIN)

main: $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ -DDOUBLE -DBLOOM_DEBUG -lm

.PHONY: main

clean:
	rm -f *.o $(BIN)