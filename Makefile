CC := gcc
CFLAGS := -O2 -Wall -Wno-unused-result -s
LDFLAGS :=

OBJS := main.o signals.o

.PHONY: all clean

all: server

server: $(OBJS)
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

clean:
	rm -f server *.o
