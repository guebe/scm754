/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"

typedef struct {
	const char *name;
	int8_t arity;
} scm_ops_t;

/* Environment which is a list of frames,
 * whereas each frame is a list of pairs (symbol . <value|procedure>).
 * This is needed to support nested environments and rebinding of variables.
 * Example: (((a . 4) (b . proc)) ((a . 5) (b . 8)))
 *
 * Note: all symbols stored in the environment are globally unique (interned in the global symbols list).
 * This is to avoid searching by string-compare in the environment. By using
 * interned symbols we can search in the environment using numeric equality
 * instead of string comparison. Even though symbols are globally unique, each
 * symbol can have a different binding in every environment frame! */
scm_obj_t scm_interaction_environment;

/* Symbol table: list of interned symbols
 * (a b c) (cons a (cons b (cons c '()))) */
scm_obj_t scm_symbols = SCM_NIL;

static const scm_ops_t ops[] =
{
	[SCM_OP_IF] = { "if", -1 },
	[SCM_OP_OR] = { "or", -1 },
	[SCM_OP_LET] = { "let", -1 },
	[SCM_OP_AND] = { "and", -1 },
	[SCM_OP_LET_STAR] = { "let*", -1 },
	[SCM_OP_QUOTE] = { "quote", -1 },
	[SCM_OP_LAMBDA] = { "lambda", -1 },
	[SCM_OP_DEFINE] = { "define", -1 },

	[SCM_OP_MUL] = { "*", -1 },
	[SCM_OP_ADD] = { "+", -1 },
	[SCM_OP_SUB] = { "-", -1 },
	[SCM_OP_DIV] = { "/", -1 },
	[SCM_OP_LT] = { "<", -1 },
	[SCM_OP_LE] = { "<=", -1 },
	[SCM_OP_NUMBER_EQ] = { "=", -1 },
	[SCM_OP_GT] = { ">", -1 },
	[SCM_OP_GE] = { ">=", -1 },
	[SCM_OP_IS_BOOLEAN] = { "boolean?", 1 },
	[SCM_OP_CAR] = { "car", 1 },
	[SCM_OP_CDR] = { "cdr", 1 },
	[SCM_OP_IS_CHAR] = { "char?", 1 },
	[SCM_OP_CONS] = { "cons", 2 },
	[SCM_OP_DISPLAY] = { "display", 1 },
	[SCM_OP_IS_EOF_OBJECT] = { "eof-object?", 1 },
	[SCM_OP_IS_EQ] = { "eq?", 2 },
	[SCM_OP_LENGTH] = { "length", 1 },
	[SCM_OP_LOAD] = { "load", 1 },
	[SCM_OP_MODULO] = { "modulo", 2 },
	[SCM_OP_NEWLINE] = { "newline", 0 },
	[SCM_OP_IS_NULL] = { "null?", 1 },
	[SCM_OP_IS_NUMBER] = { "number?", 1 },
	[SCM_OP_NUMBER_TO_STRING] = { "number->string", 1 },
	[SCM_OP_IS_PAIR] = { "pair?", 1 },
	[SCM_OP_IS_PROCEDURE] = { "procedure?", 1 },
	[SCM_OP_QUOTIENT] = { "quotient", 2 },
	[SCM_OP_SET_CAR] = { "set-car!", 2 },
	[SCM_OP_SET_CDR] = { "set-cdr!", 2 },
	[SCM_OP_STRING_EQ] = { "string=?", -1 },
	[SCM_OP_IS_STRING] = { "string?", 1 },
	[SCM_OP_STRING_LENGTH] = { "string-length", 1 },
	[SCM_OP_SUBSTRING] = { "substring", -1 },
	[SCM_OP_IS_SYMBOL] = { "symbol?", 1 },
	[SCM_OP_WRITE] = { "write", 1 },
	[SCM_OP_IS_ZERO] = { "zero?", 1 },
};

extern scm_obj_t scm_intern(scm_obj_t symbol)
{
	scm_obj_t x, y, z, w;
	const char *name, *name2;

	x = scm_symbols;
	assert(scm_is_symbol(symbol));
	z = scm_symbol_to_string(symbol);
	name = scm_string_value(z);
	while (scm_is_pair(x)) {
		y = scm_car(x);
		assert(scm_is_symbol(y));
		w = scm_symbol_to_string(y);
		name2 = scm_string_value(w);
		if (strcmp(name, name2) == 0) return y;
		x = scm_cdr(x);
	}
	scm_symbols = scm_cons(symbol, scm_symbols);

	return symbol;
}

extern const char *scm_procedure_string(scm_obj_t proc)
{
	uint32_t id = scm_procedure_id(proc);
	return ops[id].name;
}

extern int8_t scm_procedure_arity(scm_obj_t proc)
{
	uint32_t id = scm_procedure_id(proc);
	return ops[id].arity;
}

extern scm_obj_t scm_environment_create(void)
{
	scm_gc_init();

	assert((SCM_OP_PROCEDURE_LAST + 1) == sizeof(ops)/sizeof(ops[0]));

	/* pre-intern all operations (special forms and procedures) to get
	 * stable index and O(1) lookup during eval */
	for (size_t i = 0; i < sizeof(ops)/sizeof(ops[0]); i++)
		(void)scm_string_to_symbol(scm_string(ops[i].name, strlen(ops[i].name)));

	/* initial environment is empty*/
	return scm_cons(scm_nil(), scm_nil());
}

extern scm_obj_t scm_environment_lookup(scm_obj_t env, scm_obj_t symbol)
{
	scm_obj_t x, y;

	assert(scm_is_symbol(symbol));
	while (scm_is_pair(env)) {
		x = scm_car(env);
		while (scm_is_pair(x)) {
			y = scm_car(x);
			assert(scm_is_pair(y));
			if (scm_car(y) == symbol) return scm_cdr(y);
			x = scm_cdr(x);
		}
		env = scm_cdr(env);
	}

	/* check if its a pre-interned procedure */
	uint32_t id = scm_procedure_id(symbol);
	if ((id >= SCM_OP_PROCEDURE_FIRST) && (id <= SCM_OP_PROCEDURE_LAST))
		return scm_procedure(id);

	return scm_error("unbound variable %s", scm_string_value(scm_symbol_to_string(symbol)));
}

extern void scm_environment_define(scm_obj_t env, scm_obj_t symbol, scm_obj_t value)
{
	assert(scm_is_pair(env));
	assert(scm_is_symbol(symbol));
	scm_set_car(env, scm_cons(scm_cons(symbol, value), scm_car(env)));
}

extern scm_obj_t scm_environment_extend(scm_obj_t env, scm_obj_t params, scm_obj_t args)
{
	if (scm_is_null(params) && scm_is_null(args))
		return env;

	if (!scm_is_pair(params) || !scm_is_pair(args))
		return scm_error("environment: params or args missing");

	scm_obj_t frame = scm_nil();
	while (scm_is_pair(params) && scm_is_pair(args)) {
		frame = scm_cons(scm_cons(scm_car(params), scm_car(args)), frame);
		params = scm_cdr(params);
		args   = scm_cdr(args);
	}

	if (!scm_is_null(params) || !scm_is_null(args))
		return scm_error("environment: parameter/argument mismatch");

	return scm_cons(frame, env);
}
