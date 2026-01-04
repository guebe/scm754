
CFLAGS = -Wall -Wextra -Werror -O3 -fjump-tables
#CFLAGS = -Wall -Wextra -Werror -g -O0

all: scheme

scheme: main.o read.o display.o cell.o
	${CC} ${CFLAGS} -o $@ $^

clean:
	rm -f scheme *.o

main.o: main.c read.h display.h
read.o: read.c read.h box.h cell.h pair.h
display.o: display.c display.h box.h cell.h
cell.o: cell.c cell.h

