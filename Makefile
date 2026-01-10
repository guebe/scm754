# (c) guenter.ebermann@htl-hl.ac.at

SRC = error.c number.c pair.c port.c read.c
SRC_SCHEME = $(SRC) write.c main.c
SRC_FUZZ = $(SRC) fuzz.c

.PHONY: all debug test clean analyze fuzz_blackbox

all: scheme

debug: scheme_debug

test: test_read.test fuzz_whitebox fuzz_blackbox analyze

clean:
	rm -f scheme scheme_debug fuzz_whitebox *.out *.plist

analyze:
	clang --analyze $(SRC_SCHEME)

fuzz_blackbox:
	./fuzz.sh

scheme: $(SRC_SCHEME) scheme.h
	clang -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Wstrict-prototypes -Werror -O3 -fjump-tables -o $@ $(SRC_SCHEME)

scheme_debug: $(SRC_SCHEME) scheme.h
	clang -O0 -g -fsanitize=address,undefined -o $@ $(SRC_SCHEME)

fuzz_whitebox: $(SRC_FUZZ) scheme.h
	clang -O1 -g -fsanitize=fuzzer,address,undefined -o $@ $(SRC_FUZZ)

# test golden reference and round-trip invariant
%.test: %.scm
	./scheme < $< > $*.out
	diff $*.ref $*.out
	sed 's/> //' $*.out > $*.scm.out
	./scheme < $*.scm.out > $*.scm.out.out
	diff $*.out $*.scm.out.out
