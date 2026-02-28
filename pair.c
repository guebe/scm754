/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

scm_pair_t cell[SCM_CELL_NUM];

static uint64_t mark_bits[SCM_CELL_NUM/64];
uint64_t free_bits[SCM_CELL_NUM/64];
size_t free_index;
_Static_assert(SCM_CELL_NUM % 64 == 0, "SCM_CELL_NUM must be multiple of 64");

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
	scm_gc_string_init();
	scm_gc_init2(free_bits, sizeof(free_bits), &free_index, mark_bits);
	memset(stack, 0, sizeof(stack));
	stack_index = 0;
}

static void mark(scm_obj_t obj)
{
tail_call:
	if (scm_is_pair(obj) || scm_is_closure(obj)) {
		size_t i = (uint32_t)obj;
		assert(i < SCM_CELL_NUM);
		if (mark_bits[i/64] & (1ULL << (i%64))) return;
		mark_bits[i/64] |= (1ULL << (i%64));
		mark(cell[i].car);
		obj = cell[i].cdr;
		goto tail_call;
	}
	else if (scm_is_string(obj) || scm_is_symbol(obj)) {
		scm_gc_string_mark(obj);
	}
}

extern void scm_gc_collect(void)
{
	static int i = 0;
	if (i++ % 3000 == 0) {
		for (size_t j = 0; j < stack_index; j++) {
			mark(*stack[j]);
		}
		scm_gc_sweep(free_bits, sizeof(free_bits), &free_index, mark_bits);
		scm_gc_string_sweep();
	}
}
