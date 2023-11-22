CC = gcc
DEBUG = -g
DEFINES =
CFLAGS = $(DEBUG) -I/u/rchaney/Classes/cs333/Labs/Lab3 -Wall -Wextra -Wshadow -Wunreachable-code \
	 -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes \
	 -Wdeclaration-after-statement -Wno-return-local-addr -Wunsafe-loop-optimizations \
	 -Wuninitialized -Werror -Wno-unused-parameter -pthread
LDFLAGS = -lcrypt
INCLUDES = /u/rchaney/Classes/cs333/Labs/Lab3/thread_crypt.h
PROG = thread_crypt

all: $(PROG)

$(PROG): $(PROG).o
	$(CC) -o $@ $^ $(LDFLAGS)

$(PROG).o: $(PROG).c $(INCLUDES)
	$(CC) -c $(CFLAGS) -o $@ $<

tar:
	tar cvfa Lab3_${LOGNAME}.tar.gz *.[ch] [mM]akefile

clean cls:
	rm -f $(PROG) *.o *~ \#*

