# (c) guenter.ebermann@htl-hl.ac.at
SRC = number.c pair.c port.c read.c write.c environment.c procedures.c eval.c string.c
CC = clang
CFLAGS = -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Wstrict-prototypes -Wsign-compare -Wformat-security -Wmisleading-indentation -Wnonnull -Wold-style-definition -Wnested-externs -Werror -fjump-tables

all: scm754 scm754 scm754-debug fuzzer test test-r7rs fuzz analyze tidy

clean:
	rm -f scm754 scm754-debug fuzzer *.out *.plist

scm754: $(SRC) error.c main.c scm754.h
	$(CC) $(CFLAGS) -DNDEBUG -O2 -flto -g -o $@ $(SRC) error.c main.c -lm

scm754-debug: $(SRC) error.c main.c scm754.h
	$(CC) $(CFLAGS) -O1 -g -fsanitize=address,undefined -o $@ $(SRC) error.c main.c -lm

fuzzer: $(SRC) fuzzer.c scm754.h
	$(CC) $(CFLAGS) -O1 -g -fsanitize=fuzzer,address,undefined -o $@ $(SRC) fuzzer.c -lm

test: test.scm
	./scm754 $< > test.out
	@if [ -s test.out ]; then cat test.out; exit 1; fi

test-r7rs: test-r7rs.scm
	./scm754 $< > test-r7rs.out
	@if [ -s test-r7rs.out ]; then cat test-r7rs.out; exit 1; fi

fuzz:
	./fuzzer -max_total_time=3 -verbosity=0 -dict=scheme.dict corpus

analyze:
	clang --analyze $(SRC) error.c main.c

tidy:
	clang-tidy $(SRC) error.c main.c -checks=misc-unused-functions,misc-unused-parameters,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,-clang-analyzer-valist.Uninitialized -- -I.
