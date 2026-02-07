/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

#define SCM_STRING_NUM 2048U

typedef struct
{
	char *string;
	uint32_t next;
	uint8_t mark;
} scm_string_t;

static scm_string_t strings[SCM_STRING_NUM];
static uint32_t head = 0;

extern void scm_gc_string_mark(scm_obj_t obj)
{
	assert(scm_is_string(obj) || scm_is_symbol(obj));
	uint32_t i = (uint32_t)obj;

	assert(i < SCM_STRING_NUM);
	assert(strings[i].string != NULL);

	strings[i].mark = 1;
}

extern void scm_gc_string_sweep(void)
{
	uint32_t tail = UINT32_MAX;
	for (uint32_t i = 0; i < SCM_STRING_NUM; i++) {
		scm_string_t *x = &strings[i];
		if (!x->mark) {
			free(x->string);
			x->string = NULL;
			x->next = tail;
			tail = i;
		}
		x->mark = 0;
	}
	head = tail;
}

extern void scm_gc_string_init(void)
{
	for (uint32_t i = 0; i < SCM_STRING_NUM; i++) {
		strings[i].next = ((i + 1) < SCM_STRING_NUM) ? i + 1 : UINT32_MAX;
		strings[i].string = NULL;
	}
	head = 0;
}

extern char *scm_string_value(scm_obj_t string)
{
	if (!scm_is_string(string)) {
		(void)scm_error("error: scm_string_value: not a string");
		return "<not a string>";
	}

	uint32_t i = (uint32_t)string;

	assert(i < SCM_STRING_NUM);
	assert(strings[i].string != NULL);

	return strings[i].string;
}

extern scm_obj_t scm_string(const char *string, size_t k)
{
	if (head == UINT32_MAX) return scm_error("out of string memory");

	char *cstr = strndup(string, k);
	if (cstr == NULL) return scm_error("string allocation failed");

	uint32_t i = head;

	assert(i < SCM_STRING_NUM);
	assert(strings[i].string == NULL);

	strings[i].string = cstr;
	head = strings[i].next;

	return SCM_STRING | i;
}
