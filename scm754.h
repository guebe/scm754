/* (c) guenter.ebermann@htl-hl.ac.at */

#ifndef __SCM754_H__
#define __SCM754_H__

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t scm_obj_t;

/* tags */
#define SCM_MASK         0xffff000000000000
/* do not use: -inf      0xfff0000000000000 */
#define SCM_NIL          0xfff1000000000000
#define SCM_TRUE         0xfff2000000000000
#define SCM_FALSE        0xfff3000000000000
#define SCM_EOF          0xfff4000000000000
#define SCM_DOT          0xfff5000000000000
#define SCM_RPAREN       0xfff6000000000000
#define SCM_UNSPECIFIED  0xfff7000000000000
/* do not use: -nan      0xfff8000000000000 */
#define SCM_ERROR        0xfff9000000000000
#define SCM_SYMBOL       0xfffa000000000000
#define SCM_STRING       0xfffb000000000000
#define SCM_PAIR         0xfffc000000000000
#define SCM_CHAR         0xfffd000000000000
/* procedure is a built-in - evaluate arguments then apply procedure - the
 * caller already evaluated all arguments - the callee takes already evaluated
 * arguments */
#define SCM_PROCEDURE    0xfffe000000000000
/* closure is a user defined function (lambda or define special form) - index
 * to cell with a pair ((variables) . (body)) stored in the environment */
#define SCM_CLOSURE      0xffff000000000000

/* primitives - special forms which control evaluation and thus are kept
 * separate from procedures. Each primitive itself (callee) evaluates its
 * arguments according to special rules defined by the specification. Those
 * primitives are interned to speed-up evaluation but _not_ stored in any
 * environment. */
extern scm_obj_t scm_if;
extern scm_obj_t scm_quote;
extern scm_obj_t scm_lambda;
extern scm_obj_t scm_define;

extern scm_obj_t scm_interaction_environment;
extern scm_obj_t scm_symbols;
extern FILE *scm_current_input_port;

typedef enum {
	/* sort enum tags by arity - this helps compiler-optimizing the eval function */

	/* arity: 0 */
	SCM_PROCEDURE_NEWLINE = 1,

	/* arity: 1 */
	SCM_PROCEDURE_CAR,
	SCM_PROCEDURE_CDR,
	SCM_PROCEDURE_IS_PROCEDURE,
	SCM_PROCEDURE_IS_NULL,
	SCM_PROCEDURE_IS_BOOLEAN,
	SCM_PROCEDURE_IS_EOF_OBJECT,
	SCM_PROCEDURE_IS_SYMBOL,
	SCM_PROCEDURE_IS_STRING,
	SCM_PROCEDURE_IS_PAIR,
	SCM_PROCEDURE_IS_CHAR,
	SCM_PROCEDURE_IS_NUMBER,
	SCM_PROCEDURE_LENGTH,
	SCM_PROCEDURE_DISPLAY,
	SCM_PROCEDURE_LOAD,
	SCM_PROCEDURE_IS_ZERO,

	/* arity: 2 */
	SCM_PROCEDURE_IS_EQ,
	SCM_PROCEDURE_CONS,
	SCM_PROCEDURE_SET_CAR,
	SCM_PROCEDURE_SET_CDR,
	SCM_PROCEDURE_MODULO,
	SCM_PROCEDURE_QUOTIENT,

	/* arity: variable length */
	SCM_PROCEDURE_ADD,
	SCM_PROCEDURE_SUB,
	SCM_PROCEDURE_MUL,
	SCM_PROCEDURE_DIV,
	SCM_PROCEDURE_WRITE,
	SCM_PROCEDURE_NUMERIC_EQUAL,
} scm_procedure_t;

/* type predicates */
static inline bool scm_is_null(scm_obj_t obj)         { return (obj & SCM_MASK) == SCM_NIL; }
static inline bool scm_is_boolean(scm_obj_t obj)      { return ((obj & SCM_MASK) == SCM_TRUE) || ((obj & SCM_MASK) == SCM_FALSE); }
static inline bool scm_is_eof_object(scm_obj_t obj)   { return (obj & SCM_MASK) == SCM_EOF; }
static inline bool scm_is_dot(scm_obj_t obj)          { return (obj & SCM_MASK) == SCM_DOT; }
static inline bool scm_is_rparen(scm_obj_t obj)       { return (obj & SCM_MASK) == SCM_RPAREN; }
static inline bool scm_is_unspecified(scm_obj_t obj)  { return (obj & SCM_MASK) == SCM_UNSPECIFIED; }
static inline bool scm_is_error(scm_obj_t obj)        { return (obj & SCM_MASK) == SCM_ERROR; }
static inline bool scm_is_symbol(scm_obj_t obj)       { return (obj & SCM_MASK) == SCM_SYMBOL; }
static inline bool scm_is_string(scm_obj_t obj)       { return (obj & SCM_MASK) == SCM_STRING; }
static inline bool scm_is_pair(scm_obj_t obj)         { return (obj & SCM_MASK) == SCM_PAIR; }
static inline bool scm_is_char(scm_obj_t obj)         { return (obj & SCM_MASK) == SCM_CHAR; }
static inline bool scm_is_procedure(scm_obj_t obj)    { return (obj & SCM_MASK) == SCM_PROCEDURE; }
static inline bool scm_is_closure(scm_obj_t obj)      { return (obj & SCM_MASK) == SCM_CLOSURE; }
static inline bool scm_is_number(scm_obj_t obj)
{
	scm_obj_t exp = (obj >> 52) & 0x7FF;
	scm_obj_t tag = (obj >> 48) & 0xF;
	return (exp != 0x7FF) || (((tag == 0) || (tag == 8)) && (exp == 0x7FF));
}

/* accessors */
static inline bool scm_boolean_value(scm_obj_t obj)         { return obj != SCM_FALSE; }
static inline double scm_number_value(scm_obj_t number)      { double d; memcpy(&d, &number, sizeof d); return d; }
static inline char scm_char_value(scm_obj_t c)               { return (char)c; }
static inline uint32_t scm_procedure_id(scm_obj_t procedure) { return (uint32_t)procedure; }
static inline uint32_t scm_closure_idx(scm_obj_t closure)    { return (uint32_t)closure; }
extern scm_obj_t scm_car(scm_obj_t pair);
extern scm_obj_t scm_cdr(scm_obj_t pair);
extern const char *scm_string_value(scm_obj_t string);
static inline size_t scm_string_length(scm_obj_t string)     { assert(scm_is_string(string)); return strlen(scm_string_value(string)); }
extern const char *scm_error_value(void);

/* mutators */
extern scm_obj_t scm_set_car(scm_obj_t pair, scm_obj_t obj);
extern scm_obj_t scm_set_cdr(scm_obj_t pair, scm_obj_t obj);

/* constructors */
static inline scm_obj_t scm_nil(void)              { return SCM_NIL; }
static inline scm_obj_t scm_boolean(bool x)        { return x?SCM_TRUE:SCM_FALSE; }
static inline scm_obj_t scm_true(void)             { return SCM_TRUE; }
static inline scm_obj_t scm_false(void)            { return SCM_FALSE; }
static inline scm_obj_t scm_eof_object(void)       { return SCM_EOF; }
static inline scm_obj_t scm_dot(void)              { return SCM_DOT; }
static inline scm_obj_t scm_rparen(void)           { return SCM_RPAREN; }
static inline scm_obj_t scm_unspecified(void)      { return SCM_UNSPECIFIED; }
static inline scm_obj_t scm_number(double number)  { scm_obj_t d; memcpy(&d, &number, sizeof d); return d; }
static inline scm_obj_t scm_char(char c)           { return SCM_CHAR | (scm_obj_t)c; }
static inline scm_obj_t scm_procedure(uint32_t id) { return SCM_PROCEDURE | id; }
static inline scm_obj_t scm_closure(uint32_t idx)  { return SCM_CLOSURE | idx; }
extern scm_obj_t scm_string_to_number(const char *string, int radix);
extern scm_obj_t scm_string(const char *string, size_t k);
extern scm_obj_t scm_cons(scm_obj_t obj1, scm_obj_t obj2);

/* primitives */
__attribute__((warn_unused_result))
extern scm_obj_t scm_error(const char *message, ...);
extern scm_obj_t scm_write(scm_obj_t obj);
extern scm_obj_t scm_display(scm_obj_t obj);
extern scm_obj_t scm_newline(void);
extern scm_obj_t scm_read(void);
extern scm_obj_t scm_load(scm_obj_t filename);
extern scm_obj_t scm_eval(scm_obj_t expr, scm_obj_t env);
extern scm_obj_t scm_apply(scm_obj_t proc, scm_obj_t args, size_t argc);
extern int scm_read_char(void);
extern int scm_peek_char(void);
extern scm_obj_t scm_environment_create(void);
extern scm_obj_t scm_environment_lookup(scm_obj_t env, scm_obj_t symbol);
extern void scm_environment_define(scm_obj_t env, scm_obj_t symbol, scm_obj_t value);
extern scm_obj_t scm_environment_extend(scm_obj_t env, scm_obj_t params, scm_obj_t args);
extern scm_obj_t scm_intern(scm_obj_t symbol);
static inline scm_obj_t scm_string_to_symbol(scm_obj_t string)
{
	if (!scm_is_string(string)) return scm_error("not a string");
	return scm_intern((string & ~SCM_MASK) | SCM_SYMBOL);
}
static inline scm_obj_t scm_symbol_to_string(scm_obj_t symbol)
{
	if (!scm_is_symbol(symbol)) return scm_error("not a symbol");
	return ((symbol & ~SCM_MASK) | SCM_STRING);
}
extern scm_obj_t scm_add(scm_obj_t args);
extern scm_obj_t scm_sub(scm_obj_t args);
extern scm_obj_t scm_mul(scm_obj_t args);
extern scm_obj_t scm_div(scm_obj_t args);
static inline scm_obj_t scm_is_eq(scm_obj_t obj1, scm_obj_t obj2) { return scm_boolean(obj1 == obj2); }
extern size_t scm_length(scm_obj_t list);
extern scm_obj_t scm_quotient(scm_obj_t a, scm_obj_t b);
extern scm_obj_t scm_modulo(scm_obj_t a, scm_obj_t b);
extern scm_obj_t scm_numeric_equal(scm_obj_t args);
extern scm_obj_t scm_is_zero(scm_obj_t z);

extern void scm_gc_init(void);
extern void scm_gc_collect(void);
extern void scm_gc_free(scm_obj_t obj);
extern bool scm_gc_contains_env(scm_obj_t obj, scm_obj_t target_env, scm_obj_t stop_at);
extern void scm_init_strings(void);
extern void scm_string_free(scm_obj_t string);
#endif
