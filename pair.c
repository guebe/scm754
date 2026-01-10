/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scheme.h"
#include <string.h>

#define SCM_CELL_MASK 0xFFFFFFFF
#define SCM_CELL_NUM 8192U

static scm_obj_t cell[SCM_CELL_NUM];
static size_t cell_idx = 0;

extern char scm_string_ref(scm_obj_t string, size_t k)
{
	size_t i, j;
       
	if (k > scm_string_length(string))
		return '\0';

	i = (string & SCM_CELL_MASK) + (k / sizeof(scm_obj_t));
	j = k % sizeof(scm_obj_t);

	return *((char *)&cell[i] + j);
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
	return scm_is_pair(pair) ? cell[i] : scm_error("car: bad argument type");
}

extern scm_obj_t scm_cdr(scm_obj_t pair)
{
	size_t i = (pair & SCM_CELL_MASK) + 1U;
	return scm_is_pair(pair) ? cell[i] : scm_error("cdr: bad argument type");
}

extern void scm_set_car(scm_obj_t pair, scm_obj_t obj)
{
	size_t i = pair & SCM_CELL_MASK;
	if (scm_is_pair(pair)) cell[i] = obj;
}

extern void scm_set_cdr(scm_obj_t pair, scm_obj_t obj)
{
	size_t i = (pair & SCM_CELL_MASK) + 1U;
	if (scm_is_pair(pair)) cell[i] = obj;
}
