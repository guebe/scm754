/* (c) guenter.ebermann@htl-hl.ac.at
 * Tiny Scheme Interpreter
 *
 * Features:
 * R7RS support
 * NaN boxing
 * garbage collection
 * tail-call optimization
 * srfi-1 library
 * readline
 * utf8
 * uses stack not the heap
 * uses indexes not pointers
 * supporting double, 52 bit integers (mantissa), null, char and boolean as immediates
 * strings and symbols are stored in-line
 *
 * Non-Features:
 * complex, rational, bytevec, vector, ports
 */

#include "scheme.h"

#include "read.c"
#include "display.c"

int main(void)
{
	while (1) {
		fputs("scheme> ", stdout);
		double x = scm_read();
		scm_display(x);
		scm_newline();
	}
}
