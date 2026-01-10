
CC = clang
#CFLAGS = -Wall -Wextra -Werror -O3 -fjump-tables
CFLAGS = -Wall -Wextra -Werror -g -O0 -fsanitize=address,undefined

all: scheme

scheme: error.c number.c pair.c port.c read.c write.c main.c scheme.h
	$(CC) $(CFLAGS) -o $@ error.c number.c pair.c port.c read.c write.c main.c

clean:
	rm -f scheme fuzz_read *.out

test: scheme test_read.test fuzz_read fuzz

# test golden reference and round-trip invariant
%.test: %.scm
	./scheme < $< > $*.out
	diff $*.ref $*.out
	sed 's/> //' $*.out > $*.scm.out
	./scheme < $*.scm.out > $*.scm.out.out
	diff $*.out $*.scm.out.out

fuzz_read: error.c number.c pair.c port.c read.c error.c number.c pair.c port.c read.c fuzz.c scheme.h
	$(CC) -fsanitize=fuzzer,address,undefined -g -O1 -o $@ error.c number.c pair.c port.c read.c fuzz.c

fuzz:
	./fuzz.sh
