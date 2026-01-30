/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

#define SCM_CELL_NUM 80000U

typedef struct
{
	scm_obj_t car;
	scm_obj_t cdr;
	uint32_t next;
	uint8_t mark;
} scm_pair_t;

static scm_pair_t cell[SCM_CELL_NUM];
static uint32_t head = 0;

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

static void mark(scm_obj_t pair)
{
	if (!scm_is_pair(pair) && !scm_is_closure(pair)) return;
	scm_obj_t i = (uint32_t)pair;
	if (cell[i].mark) return;
	cell[i].mark = 1;
	mark(cell[i].car);
	mark(cell[i].cdr);
}

static uint32_t sweep(void)
{
	uint32_t f = UINT32_MAX;
	for (uint32_t i = 0; i < SCM_CELL_NUM; i++) {
		scm_pair_t *c = &cell[i];
		if (!c->mark) {
			c->next = f;
			c->car = SCM_ERROR;
			c->cdr = SCM_ERROR;
			f = i;
			if (scm_is_string(c->car)) scm_string_free(c->car);
			else if (scm_is_symbol(c->car)) scm_string_free(scm_symbol_to_string(c->car));
			if (scm_is_string(c->cdr)) scm_string_free(c->cdr);
			else if (scm_is_symbol(c->cdr)) scm_string_free(scm_symbol_to_string(c->cdr));
		}
		else {
			c->mark = 0;
		}
	}
	return f;
}

extern void scm_gc_collect(void)
{
	mark(scm_interaction_environment);
	mark(scm_symbols);
	head = sweep();
}

extern void scm_gc_free(scm_obj_t obj)
{
	assert(scm_is_pair(obj));
	uint32_t i = (uint32_t)obj;
	cell[i].car = SCM_ERROR;
	cell[i].cdr = SCM_ERROR;
	cell[i].next = head;
	cell[i].mark = 0;
	head = i;
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
