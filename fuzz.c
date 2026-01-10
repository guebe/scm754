#include "scheme.h"
#include <stdio.h>
#include <stdint.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    FILE *mem = fmemopen((void *)data, size, "r");
    if (!mem) return 0;

    FILE *old_stdin = stdin;
    stdin = mem;
    scm_read();
    stdin = old_stdin;
    fclose(mem);
    return 0;
}
