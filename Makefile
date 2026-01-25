# (c) guenter.ebermann@htl-hl.ac.at

SRC = error.c number.c pair.c port.c read.c write.c environment.c procedures.c eval.c string.c
SRC_SCHEME = $(SRC) main.c
SRC_FUZZ = $(SRC) fuzz.c

CC = clang
CFLAGS = -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Wstrict-prototypes -Wsign-compare -Wformat-security -Wmisleading-indentation -Wnonnull -Wold-style-definition -Wnested-externs -Werror -O3 -fjump-tables
CFLAGS_EXTRA_DEBUG = -O0 -g -fsanitize=address,undefined
CFLAGS_EXTRA_FUZZ = -O1 -g -fsanitize=fuzzer,address,undefined

.PHONY: all test clean analyze fuzz-blackbox

all: scm754

test: scm754 scm754-debug test-read test-read.test test.test test-extra.test fuzz-whitebox fuzz-blackbox analyze tidy

clean:
	rm -f scm754 scm754-debug fuzz-whitebox test-read *.out *.plist

analyze:
	clang --analyze $(SRC_SCHEME)

tidy:
	clang-tidy $(SRC_SCHEME) -checks=misc-unused-functions,misc-unused-parameters,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling -- -I.

fuzz-blackbox:
	./fuzz-blackbox

scm754: $(SRC_SCHEME) scm754.h
	$(CC) $(CFLAGS) -o $@ $(SRC_SCHEME) -lm

scm754-debug: $(SRC_SCHEME) scm754.h
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA_DEBUG) -o $@ $(SRC_SCHEME) -lm

fuzz-whitebox: $(SRC_FUZZ) scm754.h
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA_FUZZ) -o $@ $(SRC_FUZZ)

test-read: $(SRC_SCHEME) scm754.h
	$(CC) $(CFLAGS) -DSCM_NO_EVAL -o $@ $(SRC_SCHEME) -lm

# test (read) and its round-trip invariant
test-read.test: test-read.scm
	./test-read < $< > test-read.out
	diff test-read.ref test-read.out
	sed 's/> //' test-read.out > test-read.scm.out
	./test-read < test-read.scm.out > test-read.scm.out.out
	diff test-read.out test-read.scm.out.out

# test read/eval/apply cycle
test.test: test.scm
	./scm754 < $< > test.out
	diff test.ref test.out

test-extra.test: test-extra.scm
	./scm754 < $< > test-extra.out
	diff test-extra.ref test-extra.out

