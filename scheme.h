/* (c) guenter.ebermann@htl-hl.ac.at
 * Tiny Scheme Interpreter
 *
 * Scheme procedures
 */

#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <inttypes.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"

static inline double scm_cons(double a, double b)
{
	size_t tmp, i;

	tmp = i = scm_i;
	if (i + 2 >= SCM_CELL_NUM) errx(EXIT_FAILURE, "out of memory\n");
	scm_cell[i++] = a;
	scm_cell[i++] = b;
	scm_i = i;
	return scm_box(SCM_LIST, tmp);
}

static inline double scm_car(double a)
{
	uint64_t value, x;
	unsigned int tag;

	x = scm_unbox(a);
	tag = scm_get_tag(x);
	value = scm_value(x);

	if (tag != SCM_LIST)
		errx(EXIT_FAILURE, "car: bad argument type");

	return scm_cell[value];
}

static inline double scm_cdr(double a)
{
	uint64_t value, x;
	unsigned int tag;

	x = scm_unbox(a);
	tag = scm_get_tag(x);
	value = scm_value(x);

	if (tag != SCM_LIST)
		errx(EXIT_FAILURE, "car: bad argument type");

	return scm_cell[value+1];
}

static inline void scm_set_car(double a, double b)
{
	uint64_t i = scm_value(scm_unbox(a));
	scm_cell[i] = b;
}

static inline void scm_set_cdr(double a, double b)
{
	uint64_t i = scm_value(scm_unbox(a));
	scm_cell[i+1] = b;
}

extern void scm_display(double obj);
extern void scm_newline(void);
extern double scm_read(void);

#endif

