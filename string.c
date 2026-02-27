/* (c) guenter.ebermann@htl-hl.ac.at */

/*
 * Custom string and symbol implementation for scm754.
 *
 * - Strings and symbols are handled uniformly.
 * - The allocator uses a fixed-size, memory-contiguous pool.
 * - The garbage collection algorithm is mark and sweep; no per-string reference counting.
 *   It uses a bitvector for marking and for freelist handling. This leads to
 *   simple and fast code (the compiler generates SIMD instructions).
 * - Each string is a heap allocated (malloc) null-terminated C-string; no
 *   explicit length field is stored. This is different from traditional
 *   scheme implementations.
 */
#include "scm754.h"

#define SCM_STRING_NUM 2048U
static char *strings[SCM_STRING_NUM];

static uint64_t mark_bits[SCM_STRING_NUM/64];
static uint64_t string_free_bits[SCM_STRING_NUM/64];
static size_t string_free_index;
_Static_assert(SCM_STRING_NUM % 64 == 0, "SCM_STRING_NUM must be multiple of 64");

extern void scm_gc_string_mark(scm_obj_t obj)
{
	assert(scm_is_string(obj) || scm_is_symbol(obj));
	size_t i = (uint32_t)obj;
	assert(i < SCM_STRING_NUM);
	mark_bits[i/64] |= (1ULL << (i%64));
}

extern void scm_gc_string_sweep(void)
{
	for (size_t i = 0; i < SCM_STRING_NUM/64; i++) {
		string_free_bits[i] = ~mark_bits[i];
		mark_bits[i] = 0;
	}
	string_free_index = 0;
}

extern void scm_gc_string_init(void)
{
	for (size_t i = 0; i < SCM_STRING_NUM; i++) {
		strings[i] = NULL;
	}
	memset(string_free_bits, 0xFF, sizeof(string_free_bits));
	memset(mark_bits, 0, sizeof(mark_bits));
	string_free_index = 0;
}

extern void scm_gc_string_free(void)
{
	for (size_t i = 0; i < SCM_STRING_NUM; i++)
		free(strings[i]);
}

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

static inline size_t alloc_string(void) {
	for (size_t i = string_free_index; i < SCM_STRING_NUM/64; i++) {
		if (string_free_bits[i]) {
			int bit = __builtin_ctzll(string_free_bits[i]); /* count trailing zeros */
			string_free_bits[i] &= string_free_bits[i] - 1;
			string_free_index = i;
			return i * 64 + (size_t)bit;
		}
	}
	scm_fatal("out of string memory");
}

extern scm_obj_t scm_string(const char *string, size_t k)
{
	size_t i = alloc_string();
	assert(i < SCM_STRING_NUM);

	char *s = strings[i];

	s = realloc(s, k+1);
	if (s == NULL) scm_fatal("string allocation failed");
	memcpy(s, string, k);
	s[k] = '\0';

	strings[i] = s;
	return SCM_STRING | i;
}
