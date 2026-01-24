/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCM_CELL_MASK 0xFFFFFFFF
#define SCM_CELL_NUM 1000U

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
	for (uint32_t i = 0; i < SCM_CELL_NUM; i++) {
		scm_pair_t *c = &cell[i];
		c->next = (i != (SCM_CELL_NUM - 1)) ? i + 1 : SCM_CELL_MASK;
		c->car = SCM_ERROR;
		c->cdr = SCM_ERROR;
		c->mark = 0;
	}
	head = 0;
}

static void mark(scm_obj_t pair)
{
	if (!scm_is_pair(pair) && !scm_is_closure(pair)) return;
	scm_pair_t *c = cell + (pair & SCM_CELL_MASK);
	if (c->mark) return;
	c->mark = 1;
	mark(c->car);
	mark(c->cdr);
}

static inline char *to_string(scm_obj_t string) { return (char *)(string & (~SCM_MASK)); }

static uint32_t sweep(void)
{
	uint32_t f = SCM_CELL_MASK;
	for (uint32_t i = 0; i < SCM_CELL_NUM; i++) {
		scm_pair_t *c = cell + i;
		if (!c->mark) {
			c->next = f;
			c->car = SCM_ERROR;
			c->cdr = SCM_ERROR;
			f = i;
#if 0
			if (scm_is_string(c->car)) free(to_string(c->car));
			else if (scm_is_symbol(c->car)) free(to_string(scm_symbol_to_string(c->car)));
			if (scm_is_string(c->cdr)) free(to_string(c->cdr));
			else if (scm_is_symbol(c->cdr)) free(to_string(scm_symbol_to_string(c->cdr)));
#endif
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
	uint32_t i = obj & SCM_CELL_MASK;
	scm_pair_t *c = &cell[i];
	c->car = SCM_ERROR;
	c->cdr = SCM_ERROR;
	c->next = head;
	c->mark = 0;
	head = i;
}

extern bool scm_gc_contains_env(scm_obj_t obj, scm_obj_t target_env, scm_obj_t stop_at)
{
	if (obj == stop_at) return false;
	else if (scm_is_closure(obj)) {
		scm_obj_t closure_env = scm_car((obj & ~SCM_MASK) | SCM_PAIR);
		return closure_env == target_env;
	}
	else if (scm_is_pair(obj)) {
		return scm_gc_contains_env(scm_car(obj), target_env, stop_at) ||
			scm_gc_contains_env(scm_cdr(obj), target_env, stop_at);
	}
	else return false;
}

extern const char *scm_string_value(scm_obj_t string)
{
	if (!scm_is_string(string)) {
		puts("error: scm_string_value: not a string");
		return "<not a string>";
	}
	return to_string(string);
}

extern scm_obj_t scm_string(const char *string)
{
	return SCM_STRING | ((uintptr_t)strdup(string) & (~SCM_MASK));
}

extern scm_obj_t scm_cons(scm_obj_t obj1, scm_obj_t obj2)
{
	if (head == SCM_CELL_MASK) return scm_error("out of memory");

	size_t i = head;
	scm_pair_t *c = &cell[i];
	c->car = obj1;
	c->cdr = obj2;
	head = c->next;
	return SCM_PAIR | i;
}

extern scm_obj_t scm_car(scm_obj_t pair)
{
	if (!scm_is_pair(pair)) return scm_error("car: not a pair");
	scm_obj_t car = cell[pair & SCM_CELL_MASK].car;
	assert(car != SCM_ERROR);
	return car;
}

extern scm_obj_t scm_cdr(scm_obj_t pair)
{
	if (scm_is_error(pair)) return scm_error("cdr: memory error");
	scm_obj_t cdr = cell[pair & SCM_CELL_MASK].cdr;
	assert(cdr != SCM_ERROR);
	return cdr;
}

extern scm_obj_t scm_set_car(scm_obj_t pair, scm_obj_t obj)
{
	if (scm_is_error(pair)) return scm_error("set-car!: memory error");
	size_t i = pair & SCM_CELL_MASK;
	assert(cell[i].car != SCM_ERROR);
	cell[i].car = obj;
	return scm_unspecified();
}

extern scm_obj_t scm_set_cdr(scm_obj_t pair, scm_obj_t obj)
{
	if (scm_is_error(pair)) return scm_error("set-cdr!: memory error");
	size_t i = pair & SCM_CELL_MASK;
	assert(cell[i].cdr != SCM_ERROR);
	cell[i].cdr = obj;
	return scm_unspecified();
}
