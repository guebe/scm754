/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

#define SCM_STRING_NUM 1024U

typedef struct
{
	char *string;
	uint32_t next;
} scm_string_t;

static scm_string_t strings[SCM_STRING_NUM];
static uint32_t head = 0;

extern void scm_string_init(void)
{
	for (uint32_t i = 0; i < SCM_STRING_NUM; i++) {
		strings[i].string = NULL;
		strings[i].next = ((i + 1) < SCM_STRING_NUM) ? i + 1 : UINT32_MAX;
	}
	head = 0;
}

extern const char *scm_string_value(scm_obj_t string)
{
	if (!scm_is_string(string)) {
		puts("error: scm_string_value: not a string");
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

extern void scm_string_free(scm_obj_t string)
{
	assert(scm_is_string(string));

	uint32_t i = (uint32_t)string;

	assert(i < SCM_STRING_NUM);
	assert(strings[i].string != NULL);

	free(strings[i].string);
	strings[i].string = NULL;
	strings[i].next = head;
	head = i;
}
