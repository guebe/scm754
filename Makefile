
CFLAGS = -Wall -Wextra -Werror -O3 -fjump-tables
#CFLAGS = -Wall -Wextra -Werror -g -O0

all: scheme

scheme: main.o read.o display.o cell.o
	${CC} ${CFLAGS} -o $@ $^

clean:
	rm -f scheme *.o *.out

main.o: main.c read.h display.h
read.o: read.c read.h box.h cell.h pair.h
display.o: display.c display.h box.h cell.h
cell.o: cell.c cell.h

.PHONY: test
test: test_read.test

%.test: %.scm
	./scheme < $< > $*.out
	diff $*.ref $*.out
	sed 's/scheme> //' $*.out > $*.scm.out
	./scheme < $*.scm.out > $*.scm.out.out
	diff $*.out $*.scm.out.out

