/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

#define SCM_STRING_NUM 2048U

typedef struct
{
	char *string;
	uint32_t next;
} scm_string_t;

static scm_string_t strings[SCM_STRING_NUM];
static uint32_t head = 0;

static uint64_t mark_bits[SCM_STRING_NUM/64];
_Static_assert(SCM_STRING_NUM % 64 == 0, "SCM_STRING_NUM must be multiple of 64");

extern void scm_gc_string_mark(scm_obj_t obj)
{
	assert(scm_is_string(obj) || scm_is_symbol(obj));
	size_t i = (uint32_t)obj;

	assert(i < SCM_STRING_NUM);
	assert(strings[i].string != NULL);

	mark_bits[i/64] |= (1ULL << (i%64));
}

extern void scm_gc_string_sweep(void)
{
	uint32_t tail = UINT32_MAX;

	for (size_t i = 0; i < (SCM_STRING_NUM/64); i++) {
		uint64_t dead = ~mark_bits[i];
		mark_bits[i] = 0;

		while (dead) {
			int j = __builtin_ctzll(dead); /* count trailing zeros */
			size_t k = i*64 + (size_t)j;
			free(strings[k].string);
			strings[k].string = NULL;
			strings[k].next = tail;
			tail = (uint32_t)k;
			dead &= (dead - 1); /* clear LSB */
		}
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
	memset(mark_bits, 0, sizeof(mark_bits));
}

extern void scm_gc_string_free(void)
{
	for (uint32_t i = 0; i < SCM_STRING_NUM; i++)
		free(strings[i].string);
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
	if (head == UINT32_MAX) scm_fatal("out of string memory");

	char *cstr = strndup(string, k);
	if (cstr == NULL) return scm_error("string allocation failed");

	uint32_t i = head;

	assert(i < SCM_STRING_NUM);
	assert(strings[i].string == NULL);

	strings[i].string = cstr;
	head = strings[i].next;

	return SCM_STRING | i;
}
