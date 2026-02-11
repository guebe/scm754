/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

scm_pair_t cell[SCM_CELL_NUM];
size_t cell_head;

static uint64_t mark_bits[SCM_CELL_NUM/64];
_Static_assert(SCM_CELL_NUM % 64 == 0, "SCM_CELL_NUM must be multiple of 64");

#define SCM_STACK_NUM  8192U
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

	for (size_t i = 0; i < SCM_CELL_NUM; i++) {
		cell[i].car_next = ((i + 1) < SCM_CELL_NUM) ? i + 1 : UINT64_MAX;
#ifndef NDEBUG
		cell[i].cdr = SCM_ERROR;
#endif
	}
	cell_head = 0;
	memset(mark_bits, 0, sizeof(mark_bits));
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
		mark(cell[i].car_next);
		obj = cell[i].cdr;
		goto tail_call;
	}
	else if (scm_is_string(obj) || scm_is_symbol(obj)) {
		scm_gc_string_mark(obj);
	}
}

static void sweep(void)
{
	size_t head = UINT64_MAX;
	for (size_t i = 0; i < (SCM_CELL_NUM/64); i++) {
		uint64_t dead = ~mark_bits[i];
		mark_bits[i] = 0;
		if (dead == 0) continue;

		if (dead == UINT64_MAX) {
			size_t k;
			for (k = i*64; k < (i*64 + 63); k++) {
				cell[k].car_next = k + 1;
#ifndef NDEBUG
				cell[k].cdr = SCM_ERROR;
#endif
			}
			cell[k].car_next = head;
#ifndef NDEBUG
			cell[k].cdr = SCM_ERROR;
#endif
			head = i*64;
			continue;
		}

		while(dead) {
			int j = __builtin_ctzll(dead); /* count trailing zeros */
			size_t k = i*64 + (size_t)j;
			cell[k].car_next = head;
#ifndef NDEBUG
			cell[k].cdr = SCM_ERROR;
#endif
			head = k;
			dead &= (dead - 1); /* clear LSB */
		}
	}
	cell_head = head;
}

extern void scm_gc_collect(void)
{
	static int i = 0;
	if (i++ % 3000 == 0) {
		for (size_t j = 0; j < stack_index; j++) {
			mark(*stack[j]);
		}
		sweep();
		scm_gc_string_sweep();
	}
}
