/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

/*
 * Custom memory arena and garbage collector implementation for scm754.
 *
 * - The allocator uses fixed-size, memory-contiguous arenas for pairs (cells),
 *   strings and symbols and vectors.
 * - The garbage collection algorithm is mark and sweep; no per-element
 *   reference counting. It uses a bitmap for marking and for free-list
 *   handling. This leads to simple and fast code (the compiler generates SIMD
 *   instructions).
 * - Pairs (cells) are stored in-line.
 * - Each string is a heap allocated (malloc) null-terminated C-string; no
 *   explicit length field is stored. This is different from traditional scheme
 *   implementations.
 * - Strings and symbols are handled uniformly.
 * - Vectors are heap allocated. An explicit length field is stored.
 */

scm_pair_t cell[SCM_CELL_NUM];
static uint64_t cell_mark_bits[SCM_CELL_NUM/64];
uint64_t cell_free_bits[SCM_CELL_NUM/64];
size_t cell_free_index;

char *strings[SCM_STRING_NUM];
static uint64_t string_mark_bits[SCM_STRING_NUM/64];
uint64_t string_free_bits[SCM_STRING_NUM/64];
size_t string_free_index;

#define SCM_STACK_NUM 8192U
static const scm_obj_t *stack[SCM_STACK_NUM];
static size_t stack_index;

extern void scm_gc_push(const scm_obj_t *obj)
{
	if (stack_index >= SCM_STACK_NUM) scm_fatal("out of stack memory");
	assert(stack[stack_index] == NULL);
	stack[stack_index++] = obj;
}

extern void scm_gc_pop(void)
{
	assert(stack_index > 0);
	stack_index--;
#ifndef NDEBUG
	stack[stack_index] = NULL;
#endif
}

extern void scm_gc_push2(const scm_obj_t *obj1, const scm_obj_t *obj2)
{
	if ((stack_index + 1) >= SCM_STACK_NUM) scm_fatal("out of stack memory");
	assert(stack[stack_index] == NULL);
	assert(stack[stack_index+1] == NULL);
	stack[stack_index] = obj1;
	stack[stack_index+1] = obj2;
	stack_index += 2;
}

extern void scm_gc_pop2(void)
{
	assert(stack_index >= 2);
#ifndef NDEBUG
	stack[stack_index-1] = NULL;
	stack[stack_index-2] = NULL;
#endif
	stack_index -= 2;
}

extern void scm_gc_init(void)
{
	for (size_t i = 0; i < SCM_STRING_NUM; i++) {
		strings[i] = NULL;
	}
	memset(cell_free_bits, 0xFF, SCM_CELL_NUM/64);
	memset(cell_mark_bits, 0, SCM_CELL_NUM/64);
	cell_free_index = 0;
	memset(string_free_bits, 0xFF, SCM_STRING_NUM/64);
	memset(string_mark_bits, 0, SCM_STRING_NUM/64);
	string_free_index = 0;
	memset(stack, 0, sizeof(stack));
	stack_index = 0;
}

extern void scm_gc_deinit(void)
{
	for (size_t i = 0; i < SCM_STRING_NUM; i++)
		free(strings[i]);
}

static void scm_gc_mark(scm_obj_t obj)
{
tail_call:
	if (scm_is_pair(obj) || scm_is_closure(obj)) {
		size_t i = (uint32_t)obj;
		assert(i < SCM_CELL_NUM);
		if (cell_mark_bits[i/64] & (1ULL << (i%64))) return;
		cell_mark_bits[i/64] |= (1ULL << (i%64));
		scm_gc_mark(cell[i].car);
		obj = cell[i].cdr;
		goto tail_call;
	}
	else if (scm_is_string(obj) || scm_is_symbol(obj)) {
		size_t i = (uint32_t)obj;
		assert(i < SCM_STRING_NUM);
		string_mark_bits[i/64] |= (1ULL << (i%64));
	}
}

static void scm_gc_sweep(void)
{
	for (size_t i = 0; i < SCM_CELL_NUM/64; i++) {
		cell_free_bits[i] = ~cell_mark_bits[i];
		cell_mark_bits[i] = 0;
	}
	cell_free_index = 0;
	for (size_t i = 0; i < SCM_STRING_NUM/64; i++) {
		string_free_bits[i] = ~string_mark_bits[i];
		string_mark_bits[i] = 0;
	}
	string_free_index = 0;
}

extern void scm_gc_collect(void)
{
	static int i = 0;
	if (i++ % 3000 == 0) {
		for (size_t j = 0; j < stack_index; j++) {
			scm_gc_mark(*stack[j]);
		}
		scm_gc_sweep();
	}
}
