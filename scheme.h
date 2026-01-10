/* (c) guenter.ebermann@htl-hl.ac.at */

#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <stdint.h>
#include <string.h>

typedef uint64_t scm_obj_t;

/* tags */
#define SCM_MASK         0xffff000000000000
/* do not use: -inf      0xfff0000000000000 */
#define SCM_EMPTY_LIST   0xfff1000000000000
#define SCM_TRUE         0xfff2000000000000
#define SCM_FALSE        0xfff3000000000000
#define SCM_EOF          0xfff4000000000000
#define SCM_DOT          0xfff5000000000000
#define SCM_RPAREN       0xfff6000000000000
/* do not use: -nan      0xfff8000000000000 */
#define SCM_ERROR        0xfff9000000000000
#define SCM_SYMBOL       0xfffa000000000000
#define SCM_STRING       0xfffb000000000000
#define SCM_PAIR         0xfffc000000000000
#define SCM_CHAR         0xfffd000000000000

#define SCM_SIZE_MASK  0xFFFF
#define SCM_SIZE_SHIFT 32U

/* type predicates */
static inline _Bool scm_is_empty_list(scm_obj_t obj)   { return (obj & SCM_MASK) == SCM_EMPTY_LIST; }
static inline _Bool scm_is_boolean(scm_obj_t obj)      { return ((obj & SCM_MASK) == SCM_TRUE) || ((obj & SCM_MASK) == SCM_FALSE); }
static inline _Bool scm_is_eof_object(scm_obj_t obj)   { return (obj & SCM_MASK) == SCM_EOF; }
static inline _Bool scm_is_dot(scm_obj_t obj)          { return (obj & SCM_MASK) == SCM_DOT; }
static inline _Bool scm_is_rparen(scm_obj_t obj)       { return (obj & SCM_MASK) == SCM_RPAREN; }
static inline _Bool scm_is_error_object(scm_obj_t obj) { return (obj & SCM_MASK) == SCM_ERROR; }
static inline _Bool scm_is_symbol(scm_obj_t obj)       { return (obj & SCM_MASK) == SCM_SYMBOL; }
static inline _Bool scm_is_string(scm_obj_t obj)       { return (obj & SCM_MASK) == SCM_STRING; }
static inline _Bool scm_is_pair(scm_obj_t obj)         { return (obj & SCM_MASK) == SCM_PAIR; }
static inline _Bool scm_is_char(scm_obj_t obj)         { return (obj & SCM_MASK) == SCM_CHAR; }

/* accessors */
static inline _Bool scm_boolean_value(scm_obj_t obj) { return obj != SCM_FALSE; }
static inline size_t scm_string_length(scm_obj_t string) { return (string >> SCM_SIZE_SHIFT) & SCM_SIZE_MASK; }
static inline double scm_number_value(scm_obj_t number) { double d; memcpy(&d, &number, sizeof d); return d; }
static inline char scm_char_value(scm_obj_t c) { return c; }
extern scm_obj_t scm_car(scm_obj_t pair);
extern scm_obj_t scm_cdr(scm_obj_t pair);
extern char scm_string_ref(scm_obj_t string, size_t k);

/* mutators */
extern void scm_set_car(scm_obj_t pair, scm_obj_t obj);
extern void scm_set_cdr(scm_obj_t pair, scm_obj_t obj);

/* constructors */
static inline scm_obj_t scm_empty_list(void) { return SCM_EMPTY_LIST; }
static inline scm_obj_t scm_boolean(_Bool x) { return x?SCM_TRUE:SCM_FALSE; }
static inline scm_obj_t scm_true(void) { return SCM_TRUE; }
static inline scm_obj_t scm_false(void) { return SCM_FALSE; }
static inline scm_obj_t scm_eof_object(void) { return SCM_EOF; }
static inline scm_obj_t scm_dot(void) { return SCM_DOT; }
static inline scm_obj_t scm_rparen(void) { return SCM_RPAREN; }
static inline scm_obj_t scm_string_to_symbol(scm_obj_t string) { return (string & ~SCM_MASK) | SCM_SYMBOL; }
static inline scm_obj_t scm_symbol_to_string(scm_obj_t symbol) { return (symbol & ~SCM_MASK) | SCM_STRING; }
static inline scm_obj_t scm_char(char c) { return SCM_CHAR | (scm_obj_t)c; }
extern scm_obj_t __attribute__((format(printf, 1, 2))) scm_error(const char *message, ...);
extern scm_obj_t scm_string_to_number(const char *string, int radix);
extern scm_obj_t scm_string(const char *string, size_t k);
extern scm_obj_t scm_cons(scm_obj_t obj1, scm_obj_t obj2);

/* primitives */
extern void scm_write(scm_obj_t obj);
extern scm_obj_t scm_read(void);
extern int scm_read_char(void);
extern int scm_peek_char(void);

#if 0
#define CLOS 6U /* closure - idx to cell with pair variables . body in env - a user defined function */
#define PRIM 7U /* primitive - idx to func ptr table - special forms - control evaluation - the primitive itself (callee) evaluates its arguments according to special rules */
#define PROC 8U /* procedure - idx to func ptr table - builtins - eval args then apply procedure - the caller already evaluated all arguments - the callee takes already evaluated arguments */
/* symbol table: list of strings key = symbol index (a b c) (cons a (cons b (cons c '()))) */
scm_obj_t symbols;
/* environments: list of list of pairs (symbol index . value|procedure) (((a . 3) (b . proc)) ((a . 5) (b . 8)) */
scm_obj_t env;
#endif

#endif

