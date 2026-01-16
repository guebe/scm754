/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"
#include <stdio.h>
#include <string.h>

#define SCM_CELL_MASK 0xFFFFFFFF
#define SCM_CELL_NUM 8192U

static scm_obj_t cell[SCM_CELL_NUM];
static size_t cell_idx = 0;

extern const char *scm_string_value(scm_obj_t string)
{
	if (scm_is_string(string)) return (const char *)&cell[string & SCM_CELL_MASK];
	else {
		fputs("error: scm_string_value called on non-string object\n", stderr);
		return "<not a string>";
	}
}

extern scm_obj_t scm_string(const char *string, size_t k)
{
	size_t i, needed, free;

	if (k > SCM_SIZE_MASK)
		return scm_error("string too long");

	i = cell_idx;
	free = SCM_CELL_NUM - i;
	needed = (k + sizeof(scm_obj_t) - 1U) / sizeof(scm_obj_t);

	if (needed > free)
		return scm_error("out of memory");

	memcpy(&cell[i], string, k);
	cell_idx = i + needed;
	return SCM_STRING | ((k & SCM_SIZE_MASK) << SCM_SIZE_SHIFT) | i;
}

extern scm_obj_t scm_cons(scm_obj_t obj1, scm_obj_t obj2)
{
	scm_obj_t pair;

	if ((cell_idx + 2U) >= SCM_CELL_NUM)
	       return scm_error("out of memory");

	pair = SCM_PAIR | cell_idx;
	cell[cell_idx++] = obj1;
	cell[cell_idx++] = obj2;

	return pair;
}

extern scm_obj_t scm_car(scm_obj_t pair)
{
	size_t i = pair & SCM_CELL_MASK;
	return scm_is_pair(pair) ? cell[i] : scm_error("car: not a pair");
}

extern scm_obj_t scm_cdr(scm_obj_t pair)
{
	size_t i = (pair & SCM_CELL_MASK) + 1U;
	return scm_is_pair(pair) ? cell[i] : scm_error("cdr: not a pair");
}

extern scm_obj_t scm_set_car(scm_obj_t pair, scm_obj_t obj)
{
	if (!scm_is_pair(pair)) return scm_error("set-car: called on non pair object");

	cell[pair & SCM_CELL_MASK] = obj;

	return scm_unspecified();
}

extern scm_obj_t scm_set_cdr(scm_obj_t pair, scm_obj_t obj)
{
	if (!scm_is_pair(pair)) return scm_error("set-cdr: called on non pair object");

	cell[(pair & SCM_CELL_MASK) + 1U] = obj;

	return scm_unspecified();
}
