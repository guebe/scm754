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

extern scm_obj_t scm_substring(scm_obj_t args)
{
	scm_obj_t a = scm_car(args);
	if (!scm_is_string(a)) return scm_error("string-copy: needs a string");
	const char *x = scm_string_value(a);

	size_t start = 0;
	size_t max = scm_string_length(a);
	size_t end;
	bool end_avail = false;

	args = scm_cdr(args);
	if (!scm_is_null(args)) {
		scm_obj_t b = scm_car(args);
		if (!scm_is_number(b)) return scm_error("substring: needs a number");
		start = scm_number_to_size(b);
		if (start == SIZE_MAX) return scm_error("substring: can't convert to size_t");

		args = scm_cdr(args);
		if (!scm_is_null(args)) {
			scm_obj_t c = scm_car(args);
			if (!scm_is_number(c)) return scm_error("substring: needs a number");
			end = scm_number_to_size(c);
			if (end == SIZE_MAX) return scm_error("substring: can't convert to size_t");

			if (!scm_is_null(scm_cdr(args))) return scm_error("substring: too many arguments");
			end_avail = true;
		}
	}

	if (!end_avail) end = max;

	if (start > end || end > max) return scm_error("string-copy: invalid arguements");

	return scm_string(x + start, end - start);
}

extern scm_obj_t scm_string_ref(scm_obj_t string, scm_obj_t k)
{
	if (!scm_is_string(string) || !scm_is_number(k)) return scm_error("string-ref: type err");

	const char *s = scm_string_value(string);
	size_t len = scm_string_length(string);
	size_t i = scm_number_to_size(k);
	if (i == SIZE_MAX) return scm_error("string-ref: can't convert to size_t");

	if (i >= len)
		return scm_error("string-ref: index %lu out of bounds (string length %lu)", i, len);

	return scm_char(s[i]);
}

extern scm_obj_t scm_string_set(scm_obj_t string, scm_obj_t k, scm_obj_t c)
{
	if (!scm_is_string(string) || !scm_is_number(k) || !scm_is_char(c)) return scm_error("string-set!: type err");

	char *s = scm_string_value(string);
	size_t len = scm_string_length(string);
	size_t i = scm_number_to_size(k);
	if (i == SIZE_MAX) return scm_error("string-set!: can't convert to size_t");

	if (i >= len)
		return scm_error("string-set!: index %lu out of bounds (string length %lu)", i, len);

	s[i] = (char)scm_char_value(c);
	return scm_unspecified();
}
