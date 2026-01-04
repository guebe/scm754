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
#include <stdio.h>

#include "read.h"

int main(void)
{
	while (1) {
		printf("scheme> ");
		fflush(stdout);
		double x = scm_read_expr();
		printf("%f\n", x);
	}
}
