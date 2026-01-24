/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"
#include <assert.h>

scm_obj_t scm_if;
scm_obj_t scm_quote;
scm_obj_t scm_lambda;
scm_obj_t scm_define;

/* Environment which is a list of frames,
 * whereas each frame is a list of pairs (symbol-index . <value|procedure>).
 * This is needed to support nested environments and rebinding of variables.
 * Example: (((a . 4) (b . proc)) ((a . 5) (b . 8)))
 *
 * Note: all symbols stored in the environment are globally unique (interned in the global_symbols list).
 * This is to avoid searching by string-compare in the environment. By using
 * interned symbols we can search in the environment using numeric equality
 * instead of string comparison. Even though symbols are globally unique, each
 * symbol can have a different binding in every environment frame! */
scm_obj_t scm_interaction_environment;

/* Symbol table: list of interned symbols
 * (a b c) (cons a (cons b (cons c '()))) */
scm_obj_t scm_symbols = SCM_NIL;

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

extern scm_obj_t scm_environment_create(void)
{
	scm_obj_t environment;

	scm_gc_init();

	scm_if     = scm_string_to_symbol(scm_string("if"));
	scm_quote  = scm_string_to_symbol(scm_string("quote"));
	scm_lambda = scm_string_to_symbol(scm_string("lambda"));
	scm_define = scm_string_to_symbol(scm_string("define"));

	environment = scm_cons(scm_nil(), scm_nil());

	scm_environment_define(environment, scm_string_to_symbol(scm_string("+")), scm_procedure(SCM_PROCEDURE_ADD));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("-")), scm_procedure(SCM_PROCEDURE_SUB));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("*")), scm_procedure(SCM_PROCEDURE_MUL));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("/")), scm_procedure(SCM_PROCEDURE_DIV));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("=")), scm_procedure(SCM_PROCEDURE_NUMERIC_EQUAL));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("car")), scm_procedure(SCM_PROCEDURE_CAR));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("cdr")), scm_procedure(SCM_PROCEDURE_CDR));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("eq?")), scm_procedure(SCM_PROCEDURE_IS_EQ));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("cons")), scm_procedure(SCM_PROCEDURE_CONS));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("write")), scm_procedure(SCM_PROCEDURE_WRITE));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("null?")), scm_procedure(SCM_PROCEDURE_IS_NULL));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("pair?")), scm_procedure(SCM_PROCEDURE_IS_PAIR));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("char?")), scm_procedure(SCM_PROCEDURE_IS_CHAR));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("modulo")), scm_procedure(SCM_PROCEDURE_MODULO));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("length")), scm_procedure(SCM_PROCEDURE_LENGTH));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("symbol?")), scm_procedure(SCM_PROCEDURE_IS_SYMBOL));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("string?")), scm_procedure(SCM_PROCEDURE_IS_STRING));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("number?")), scm_procedure(SCM_PROCEDURE_IS_NUMBER));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("boolean?")), scm_procedure(SCM_PROCEDURE_IS_BOOLEAN));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("set-car!")), scm_procedure(SCM_PROCEDURE_SET_CAR));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("set-cdr!")), scm_procedure(SCM_PROCEDURE_SET_CDR));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("quotient")), scm_procedure(SCM_PROCEDURE_QUOTIENT));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("procedure?")), scm_procedure(SCM_PROCEDURE_IS_PROCEDURE));
	scm_environment_define(environment, scm_string_to_symbol(scm_string("eof-object?")), scm_procedure(SCM_PROCEDURE_IS_EOF_OBJECT));
	return environment;
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
