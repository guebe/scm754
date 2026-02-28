/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"

extern char *scm_string_value(scm_obj_t string)
{
	if (!scm_is_string(string)) {
		(void)scm_error("error: scm_string_value: not a string");
		return "<not a string>";
	}
	size_t i = (uint32_t)string;
	assert(i < SCM_STRING_NUM);
	return strings[i];
}

extern scm_obj_t scm_string(const char *string, size_t k)
{
	size_t i = scm_gc_alloc(string_free_bits, sizeof(string_free_bits), &string_free_index);
	if (i >= SCM_STRING_NUM) scm_fatal("out of string memory");

	char *s = strings[i];

	s = realloc(s, k+1);
	if (s == NULL) scm_fatal("string allocation failed");
	memcpy(s, string, k);
	s[k] = '\0';

	strings[i] = s;
	return SCM_STRING | i;
}
