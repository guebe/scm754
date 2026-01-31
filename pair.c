/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

#define SCM_CELL_NUM  20000U
#define SCM_STACK_NUM 10000U

typedef struct
{
	scm_obj_t car;
	scm_obj_t cdr;
	uint32_t next;
	uint8_t mark;
} scm_pair_t;

static scm_pair_t cell[SCM_CELL_NUM];
static uint32_t head = 0;

static const scm_obj_t *stack[SCM_STACK_NUM];
static uint32_t stack_index = 0;

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
	stack[--stack_index] = NULL;
}

extern void scm_gc_init(void)
{
	scm_string_init();

	for (uint32_t i = 0; i < SCM_CELL_NUM; i++) {
		cell[i].next = ((i + 1) < SCM_CELL_NUM) ? i + 1 : UINT32_MAX;
		cell[i].car = SCM_ERROR;
		cell[i].cdr = SCM_ERROR;
		cell[i].mark = 0;
	}
	head = 0;
}

static void mark(scm_obj_t obj)
{
	if (scm_is_pair(obj) || scm_is_closure(obj)) {
		scm_obj_t i = (uint32_t)obj;
		if (cell[i].mark) return;
		cell[i].mark = 1;
		mark(cell[i].car);
		mark(cell[i].cdr);
	}
	else if (scm_is_string(obj) || scm_is_symbol(obj)) {
		scm_mark_string(obj);
	}
}

static void mark_stack(void)
{
	for (uint32_t i = 0; i < stack_index; i++) {
		mark(*stack[i]);
	}
}

static void sweep(void)
{
	uint32_t tail = UINT32_MAX;
	for (uint32_t i = 0; i < SCM_CELL_NUM; i++) {
		scm_pair_t *x = &cell[i];
		if (!x->mark) {
			x->car = SCM_ERROR;
			x->cdr = SCM_ERROR;
			x->next = tail;
			tail = i;
		}
		x->mark = 0;
	}
	head = tail;
}

extern void scm_gc_collect(void)
{
	static int i = 0;
	if (i++ % 1000 == 0) {
	mark(scm_interaction_environment);
	mark(scm_symbols);
	mark_stack();
	sweep();
	scm_sweep_string();
	}
}

extern scm_obj_t scm_cons(scm_obj_t obj1, scm_obj_t obj2)
{
	if (head == UINT32_MAX) return scm_error("out of memory");
	uint32_t i = head;
	cell[i].car = obj1;
	cell[i].cdr = obj2;
	head = cell[i].next;
	return SCM_PAIR | i;
}

extern scm_obj_t scm_car(scm_obj_t pair)
{
	if (!scm_is_pair(pair)) return scm_error("car: not a pair");
	scm_obj_t car = cell[(uint32_t)pair].car;
	assert(car != SCM_ERROR);
	return car;
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
	uint32_t i = (uint32_t)pair;
	assert(cell[i].car != SCM_ERROR);
	cell[i].car = obj;
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
