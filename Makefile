
CFLAGS = -Wall -Wextra -Werror -O3 -fjump-tables -fsanitize=address,undefined
#CFLAGS = -Wall -Wextra -Werror -g -O0

all: scheme

clean:
	rm -f scheme *.o *.out

scheme: scheme.c

scheme.c: display.c object.h read.c scheme.h

test: test_read.test fuzz scheme

# test golden reference and round-trip invariant
%.test: %.scm
	./scheme < $< > $*.out
	diff $*.ref $*.out
	sed 's/scheme> //' $*.out > $*.scm.out
	./scheme < $*.scm.out > $*.scm.out.out
	diff $*.out $*.scm.out.out

fuzz:
	./fuzz.sh
