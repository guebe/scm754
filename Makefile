# (c) guenter.ebermann@htl-hl.ac.at

SRC = error.c number.c pair.c port.c read.c
SRC_SCHEME = $(SRC) write.c environment.c eval.c main.c
SRC_FUZZ = $(SRC) environment.c fuzz.c

CC = clang
CFLAGS = -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Wstrict-prototypes -Wsign-compare -Wformat-security -Wmisleading-indentation -Wnonnull -Wold-style-definition -Wnested-externs -Werror -O3 -fjump-tables
CFLAGS_EXTRA_DEBUG = -O0 -g -fsanitize=address,undefined
CFLAGS_EXTRA_FUZZ = -O1 -g -fsanitize=fuzzer,address,undefined

.PHONY: all test clean analyze fuzz_blackbox

all: scm754

test: scm754 scm754_debug test_read test_read.test scm754.test fuzz_whitebox fuzz_blackbox analyze tidy

clean:
	rm -f scm754 scm754_debug fuzz_whitebox test_read *.out *.plist

analyze:
	clang --analyze $(SRC_SCHEME)

tidy:
	clang-tidy $(SRC_SCHEME) -checks=misc-unused-functions,misc-unused-parameters,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling -- -I.

fuzz_blackbox:
	./fuzz_blackbox

scm754: $(SRC_SCHEME) scm754.h
	$(CC) $(CFLAGS) -o $@ $(SRC_SCHEME)

scm754_debug: $(SRC_SCHEME) scm754.h
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA_DEBUG) -o $@ $(SRC_SCHEME)

fuzz_whitebox: $(SRC_FUZZ) scm754.h
	$(CC) $(CFLAGS) $(CFLAGS_EXTRA_FUZZ) -o $@ $(SRC_FUZZ)

test_read: $(SRC_SCHEME) scm754.h
	$(CC) $(CFLAGS) -DSCM_NO_EVAL -o $@ $(SRC_SCHEME)

# test schemes read and its round-trip invariant
%.test: %.scm
	./$* < $< > $*.out
	diff $*.ref $*.out
	sed 's/> //' $*.out > $*.scm.out
	./$* < $*.scm.out > $*.scm.out.out
	diff $*.out $*.scm.out.out

# test schemes read/eval/apply cycle
scm754.test: scm754.scm
	./scm754 < $< > scm754.out
	diff scm754.ref scm754.out
