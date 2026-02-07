/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

#define SCM_CELL_NUM  32768U
#define SCM_STACK_NUM  8192U

typedef struct
{
	scm_obj_t car_next;
	scm_obj_t cdr;
} scm_pair_t;

static scm_pair_t cell[SCM_CELL_NUM];
static size_t cell_head = 0;

static uint64_t mark_bits[SCM_CELL_NUM/64];
_Static_assert(SCM_CELL_NUM % 64 == 0, "SCM_CELL_NUM must be multiple of 64");

static const scm_obj_t *stack[SCM_STACK_NUM];
static size_t stack_index = 0;

extern bool scm_gc_push(const scm_obj_t *obj)
{
	if (stack_index >= SCM_STACK_NUM) return false;
	assert(stack[stack_index] == NULL);
	stack[stack_index++] = obj;
	return true;
}

extern void scm_gc_pop(void)
{
	assert(stack_index > 0);
	stack_index--;
#ifndef NDEBUG
	stack[stack_index] = NULL;
#endif
}

extern bool scm_gc_push2(const scm_obj_t *obj1, const scm_obj_t *obj2)
{
	if ((stack_index + 1) >= SCM_STACK_NUM) return false;
	assert(stack[stack_index] == NULL);
	assert(stack[stack_index+1] == NULL);
	stack[stack_index] = obj1;
	stack[stack_index+1] = obj2;
	stack_index += 2;
	return true;
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
	scm_string_init();

	for (size_t i = 0; i < SCM_CELL_NUM; i++) {
		cell[i].car_next = ((i + 1) < SCM_CELL_NUM) ? i + 1 : UINT64_MAX;
#ifndef NDEBUG
		cell[i].cdr = SCM_ERROR;
#endif
	}
	cell_head = 0;
	memset(mark_bits, 0, sizeof(mark_bits));
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
		scm_mark_string(obj);
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
		scm_sweep_string();
	}
}

extern scm_obj_t scm_cons(scm_obj_t obj1, scm_obj_t obj2)
{
	if (cell_head == UINT64_MAX) return scm_error("out of memory");
	size_t i = cell_head;
	cell_head = cell[i].car_next;
	cell[i].car_next = obj1;
	cell[i].cdr = obj2;
	return SCM_PAIR | i;
}

extern scm_obj_t scm_car(scm_obj_t pair)
{
	if (!scm_is_pair(pair)) return scm_error("car: not a pair");
	return cell[(uint32_t)pair].car_next;
}

extern scm_obj_t scm_cdr(scm_obj_t pair)
{
	if (!scm_is_pair(pair)) return scm_error("cdr: not a pair");
	scm_obj_t cdr = cell[(uint32_t)pair].cdr;
	assert(cdr != SCM_ERROR);
	return cdr;
}

extern scm_obj_t scm_set_car(scm_obj_t pair, scm_obj_t obj)
{
	if (!scm_is_pair(pair)) return scm_error("set-car!: not a pair");
	cell[(uint32_t)pair].car_next = obj;
	return scm_unspecified();
}

extern scm_obj_t scm_set_cdr(scm_obj_t pair, scm_obj_t obj)
{
	if (!scm_is_pair(pair)) return scm_error("set-cdr!: not a pair");
	size_t i = (uint32_t)pair;
	assert(cell[i].cdr != SCM_ERROR);
	cell[i].cdr = obj;
	return scm_unspecified();
}
