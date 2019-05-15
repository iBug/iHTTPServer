CC := gcc
CFLAGS := -O2 -Wall -Wno-unused-result
LDFLAGS :=

.PHONY: all clean

all: server

server: server.o
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

clean:
	rm -f server *.o
