
CFLAGS = -Wall -Wextra -Werror -g0 -o1

all: scheme

scheme: main.o read.o
	${CC} ${CFLAGS} -o $@ $^

clean:
	rm -f scheme *.o

main.o: main.c read.h
read.o: read.c read.h

