/* (c) guenter.ebermann@htl-hl.ac.at */

#ifndef __SCM754_H__
#define __SCM754_H__

#include <assert.h>
#include <ctype.h>
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
 * primitives are interned constants to speed-up evaluation but _not_ stored in
 * any environment. */
#define SCM_QUOTE    (SCM_SYMBOL | SCM_OP_QUOTE)
#define SCM_LAMBDA   (SCM_SYMBOL | SCM_OP_LAMBDA)

extern scm_obj_t scm_interaction_environment;
extern scm_obj_t scm_symbols;
extern FILE *scm_current_input_port;

/* primitives and procedures need a contiguous index for interning the symbols
 * with a stable id. This eases operation dispatching in eval. */
typedef enum {
	/* primitives */
	SCM_OP_IF = 0,
	SCM_OP_OR,
	SCM_OP_LET,
	SCM_OP_AND,
	SCM_OP_LET_STAR,
	SCM_OP_QUOTE,
	SCM_OP_LAMBDA,
	SCM_OP_DEFINE,

	/* procedures */
	SCM_OP_NEWLINE,
	SCM_OP_PROCEDURE_FIRST = SCM_OP_NEWLINE,
	SCM_OP_CAR,
	SCM_OP_CDR,
	SCM_OP_IS_PROCEDURE,
	SCM_OP_IS_NULL,
	SCM_OP_IS_BOOLEAN,
	SCM_OP_IS_EOF_OBJECT,
	SCM_OP_IS_SYMBOL,
	SCM_OP_IS_STRING,
	SCM_OP_IS_PAIR,
	SCM_OP_IS_CHAR,
	SCM_OP_IS_NUMBER,
	SCM_OP_LENGTH,
	SCM_OP_DISPLAY,
	SCM_OP_LOAD,
	SCM_OP_IS_ZERO,
	SCM_OP_STRING_LENGTH,
	SCM_OP_NUMBER_TO_STRING,
	SCM_OP_IS_EQ,
	SCM_OP_IS_EQV,
	SCM_OP_APPLY,
	SCM_OP_MAX,
	SCM_OP_CONS,
	SCM_OP_SET_CAR,
	SCM_OP_SET_CDR,
	SCM_OP_MODULO,
	SCM_OP_QUOTIENT,
	SCM_OP_ADD,
	SCM_OP_SUB,
	SCM_OP_MUL,
	SCM_OP_DIV,
	SCM_OP_WRITE,
	SCM_OP_NUMBER_LT,
	SCM_OP_NUMBER_GT,
	SCM_OP_NUMBER_LE,
	SCM_OP_NUMBER_GE,
	SCM_OP_NUMBER_EQ,
	SCM_OP_CHAR_LT,
	SCM_OP_CHAR_GT,
	SCM_OP_CHAR_LE,
	SCM_OP_CHAR_GE,
	SCM_OP_CHAR_EQ,
	SCM_OP_CHAR_CI_LT,
	SCM_OP_CHAR_CI_GT,
	SCM_OP_CHAR_CI_LE,
	SCM_OP_CHAR_CI_GE,
	SCM_OP_CHAR_CI_EQ,
	SCM_OP_STRING_EQ,
	SCM_OP_STRING_REF,
	SCM_OP_SUBSTRING,
	SCM_OP_PROCEDURE_LAST = SCM_OP_SUBSTRING,
} scm_op_t;

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
static inline bool scm_boolean_value(scm_obj_t obj)          { return obj != SCM_FALSE; }
static inline double scm_number_value(scm_obj_t number)      { double d; memcpy(&d, &number, sizeof d); return d; }
static inline int scm_char_value(scm_obj_t c)                { return (int)(uint32_t)c; }
static inline uint32_t scm_procedure_id(scm_obj_t procedure) { return (uint32_t)procedure; }
extern const char *scm_procedure_string(scm_obj_t proc);
extern int8_t scm_procedure_arity(scm_obj_t proc);
static inline scm_obj_t scm_closure_value(scm_obj_t closure) { return SCM_PAIR | (uint32_t)closure; }
extern scm_obj_t scm_car(scm_obj_t pair);
extern scm_obj_t scm_cdr(scm_obj_t pair);
extern const char *scm_string_value(scm_obj_t string);
static inline size_t scm_string_length(scm_obj_t string)     { assert(scm_is_string(string)); return strlen(scm_string_value(string)); }

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
static inline scm_obj_t scm_char(int c)            { return SCM_CHAR | (uint32_t)c; }
static inline scm_obj_t scm_procedure(uint32_t id) { return SCM_PROCEDURE | id; }
static inline scm_obj_t scm_closure(scm_obj_t pair){ return SCM_CLOSURE | (uint32_t)pair; }
extern scm_obj_t scm_string_to_number(const char *string, int radix);
extern scm_obj_t scm_number_to_string(scm_obj_t number);
extern scm_obj_t scm_string(const char *string, size_t k);
extern scm_obj_t scm_cons(scm_obj_t obj1, scm_obj_t obj2);

/* primitives */
__attribute__((format(printf, 1, 2)))
__attribute__((warn_unused_result))
extern scm_obj_t scm_error(const char *message, ...);
extern scm_obj_t scm_write(scm_obj_t obj);
extern scm_obj_t scm_display(scm_obj_t obj);
extern scm_obj_t scm_newline(void);
extern scm_obj_t scm_read(void);
extern scm_obj_t scm_load(const char *filename);
extern scm_obj_t scm_eval(scm_obj_t expr, scm_obj_t env);
extern scm_obj_t scm_apply(scm_obj_t proc, scm_obj_t args);
extern int scm_read_char(void);
extern int scm_peek_char(void);
extern scm_obj_t scm_environment_create(void);
extern scm_obj_t scm_environment_lookup(scm_obj_t env, scm_obj_t symbol);
extern void scm_environment_define(scm_obj_t env, scm_obj_t symbol, scm_obj_t value);
extern scm_obj_t scm_environment_extend(scm_obj_t env, scm_obj_t params, scm_obj_t args);
extern scm_obj_t scm_string_to_symbol(scm_obj_t string);
static inline scm_obj_t scm_symbol_to_string(scm_obj_t symbol)
{
	if (!scm_is_symbol(symbol)) return scm_error("not a symbol");
	return SCM_STRING | (uint32_t)symbol;
}
extern scm_obj_t scm_add(scm_obj_t args);
extern scm_obj_t scm_sub(scm_obj_t args);
extern scm_obj_t scm_mul(scm_obj_t args);
extern scm_obj_t scm_div(scm_obj_t args);
static inline scm_obj_t scm_is_eq(scm_obj_t obj1, scm_obj_t obj2) { return scm_boolean(obj1 == obj2); }
extern scm_obj_t scm_is_eqv(scm_obj_t a, scm_obj_t b);
extern size_t scm_length(scm_obj_t list);
extern scm_obj_t scm_quotient(scm_obj_t a, scm_obj_t b);
extern scm_obj_t scm_modulo(scm_obj_t a, scm_obj_t b);
extern scm_obj_t scm_is_zero(scm_obj_t z);
extern scm_obj_t scm_string_eq(scm_obj_t args);
extern scm_obj_t scm_substring(scm_obj_t args);
extern scm_obj_t scm_max(scm_obj_t args);
extern scm_obj_t scm_number_lt(scm_obj_t args);
extern scm_obj_t scm_number_gt(scm_obj_t args);
extern scm_obj_t scm_number_le(scm_obj_t args);
extern scm_obj_t scm_number_ge(scm_obj_t args);
extern scm_obj_t scm_number_eq(scm_obj_t args);
extern scm_obj_t scm_char_lt(scm_obj_t args);
extern scm_obj_t scm_char_gt(scm_obj_t args);
extern scm_obj_t scm_char_le(scm_obj_t args);
extern scm_obj_t scm_char_ge(scm_obj_t args);
extern scm_obj_t scm_char_eq(scm_obj_t args);
extern scm_obj_t scm_char_ci_lt(scm_obj_t args);
extern scm_obj_t scm_char_ci_gt(scm_obj_t args);
extern scm_obj_t scm_char_ci_le(scm_obj_t args);
extern scm_obj_t scm_char_ci_ge(scm_obj_t args);
extern scm_obj_t scm_char_ci_eq(scm_obj_t args);
extern scm_obj_t scm_string_ref(scm_obj_t str, scm_obj_t index);

extern void scm_gc_init(void);
extern void scm_gc_collect(void);
__attribute__((warn_unused_result))
extern bool scm_gc_push(const scm_obj_t *obj);
extern bool scm_gc_push2(const scm_obj_t *obj1, const scm_obj_t *obj2);
extern void scm_gc_pop2(void);
extern void scm_gc_pop(void);
extern void scm_string_init(void);
extern void scm_mark_string(scm_obj_t string);
extern void scm_sweep_string(void);
extern void scm_enable_debug(void);
extern void scm_enable_error(void);
#endif
