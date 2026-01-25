/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

#define SCM_STRING_NUM 1024U

static char *strings[SCM_STRING_NUM];
static uint32_t free_list[SCM_STRING_NUM];
static uint32_t free_count = SCM_STRING_NUM;

extern void scm_init_strings(void)
{
	for (uint32_t i = 0; i < SCM_STRING_NUM; i++)
		free_list[i] = i;
	free_count = SCM_STRING_NUM;
}

extern const char *scm_string_value(scm_obj_t string)
{
	if (!scm_is_string(string)) {
		puts("error: scm_string_value: not a string");
		return "<not a string>";
	}
	uint32_t i = (uint32_t)string;
	assert(i < SCM_STRING_NUM);
	assert(strings[i] != NULL);
	return strings[i];
}

extern scm_obj_t scm_string(const char *string, size_t k)
{
	if (free_count == 0) return scm_error("out of string memory");
	char *s = strndup(string, k);
	if (s == NULL) return scm_error("string allocation failed");
	uint32_t i = free_list[--free_count];
	strings[i] = s;
	return SCM_STRING | i;
}

extern void scm_string_free(scm_obj_t string)
{
	assert(scm_is_string(string));
	uint32_t i = (uint32_t)string;
	assert(i < SCM_STRING_NUM);
	assert(strings[i] != NULL);
	free(strings[i]);
	strings[i] = NULL;
	assert(free_count < SCM_STRING_NUM);
	free_list[free_count++] = i;
}
