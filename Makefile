# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement -Wno-return-local-addr -Wunsafe-loop-optimizations -Wuninitialized -Werror -Wno-unused-parameter -pthread
LDFLAGS = -lcrypt

all: thread_crypt

thread_crypt: thread_crypt.o
	$(CC) -o $@ $^ $(LDFLAGS)

thread_crypt.o: thread_crypt.c thread_crypt.h
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f thread_crypt thread_crypt.o

