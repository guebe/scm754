/* (c) guenter.ebermann@htl-hl.ac.at */

#undef DEBUG

#include "scm754.h"
#include <math.h>
#ifdef DEBUG
#include <stdio.h>
#endif

static scm_obj_t eval_list(scm_obj_t list, scm_obj_t environment_specifier)
{
	scm_obj_t first, rest;

	if (scm_is_empty_list(list)) return list;
	first = scm_eval(scm_car(list), environment_specifier);
	if (scm_is_error_object(first)) return first;
	rest = eval_list(scm_cdr(list), environment_specifier);
	if (scm_is_error_object(rest)) return rest;
	return scm_cons(first, rest);
}

extern scm_obj_t scm_eval(scm_obj_t expr_or_def, scm_obj_t environment_specifier)
{
	scm_obj_t op, args, evaled_args;

	if (scm_is_empty_list(expr_or_def)) {
		return scm_error("eval: can not apply empty-list-object ()");
	}
	else if (scm_is_symbol(expr_or_def)) {
		/* symbols are bound to values or procedures in the environment */
		return scm_environment_lookup(environment_specifier, scm_intern(expr_or_def));
	}
	else if (scm_is_pair(expr_or_def)) {
		op = scm_car(expr_or_def);
		args = scm_cdr(expr_or_def);

		if (scm_is_symbol(op)) {
			/* special forms */
			if (op == scm_if) {
				if (!scm_is_pair(args)) return scm_error("eval: error in if form");
				scm_obj_t cond = scm_eval(scm_car(args), environment_specifier);
				args = scm_cdr(args);
				if (!scm_is_pair(args)) return scm_error("eval: error in if form");
				scm_obj_t then = scm_car(args);
				scm_obj_t else_ = scm_cdr(args);
				if (scm_boolean_value(cond))
					return scm_eval(then, environment_specifier);
				else if (!scm_is_empty_list(else_))
					return scm_eval(scm_car(else_), environment_specifier);
				else
					return scm_unspecified();
			}
			if (op == scm_quote) {
				return scm_is_empty_list(scm_cdr(args)) ? scm_car(args) : scm_error("eval: bad quote form");
			}
			if (op == scm_define) {
				scm_obj_t var = scm_car(args);

				/* de-sugar define special form to lambda (define (name params) body)*/
				if (scm_is_pair(var) && scm_is_symbol(scm_car(var))) {
					scm_obj_t variable = scm_car(var);
					scm_obj_t formals = scm_cdr(var);
					scm_obj_t body = scm_cdr(args);
					scm_obj_t value = scm_eval(scm_cons(scm_lambda, scm_cons(formals, body)), environment_specifier);
					if (scm_is_error_object(value)) return value;
					scm_environment_define(environment_specifier, scm_intern(variable), value);
					return scm_unspecified();
				}

				if (!scm_is_symbol(var)) return scm_error("eval: bad define form: variable is not a symbol");
				scm_obj_t expression = scm_cdr(args);
				if (!scm_is_empty_list(scm_cdr(expression))) return scm_error("eval: bad define form: too many expressions");
				scm_obj_t value = scm_eval(scm_car(expression), environment_specifier);
				if (scm_is_error_object(value)) return value;
				scm_environment_define(environment_specifier, scm_intern(var), value);
				return scm_unspecified();
			}
			if (op == scm_lambda) {
				if (!scm_is_pair(args)) return scm_error("eval: bad lambda form: (lambda) ");
				if (!scm_is_pair(scm_car(args)) && !scm_is_empty_list(scm_car(args)) && !scm_is_symbol(scm_car(args))) return scm_error("eval: bad lambda parameter list");
				if (scm_is_empty_list(scm_cdr(args))) return scm_error("eval: lambda body missing (lambda (params))");
				/* convert the lambda to a closure for later application: capture the environment at lamda definition time */
				return scm_closure((uint32_t)scm_cons(environment_specifier, args));
			}
		}

		/* application */
		scm_obj_t proc = scm_eval(op, environment_specifier);
		if (scm_is_error_object(proc)) return proc;
		evaled_args = eval_list(args, environment_specifier);
		if (scm_is_error_object(evaled_args)) return evaled_args;
#ifdef DEBUG
		printf("; eval: calling (apply "); scm_write(proc); printf(" "); scm_write(evaled_args); printf(")\n");
#endif
		return scm_apply(proc, evaled_args);
	}
	else {
		/* self-evaluating */
		return expr_or_def;
	}
}

extern scm_obj_t scm_add(scm_obj_t args)
{
	double a = 0;
	while (scm_is_pair(args)) {
		scm_obj_t arg = scm_car(args);
		if (!scm_is_number(arg)) return scm_error("+: needs a number");
		a += scm_number_value(arg);
		args = scm_cdr(args);
	}
	return scm_number(a);
}

extern scm_obj_t scm_sub(scm_obj_t args)
{
	if (!scm_is_pair(args)) return scm_error("-: needs an argument");
	scm_obj_t arg = scm_car(args);
	if (!scm_is_number(arg)) return scm_error("-: needs a number");
	double a = scm_number_value(arg);
	args = scm_cdr(args);
	if (scm_is_empty_list(args)) return scm_number(-a);

	while (scm_is_pair(args)) {
		arg = scm_car(args);
		if (!scm_is_number(arg)) return scm_error("-: needs a number");
		a -= scm_number_value(arg);
		args = scm_cdr(args);
	}
	return scm_number(a);
}

extern scm_obj_t scm_mul(scm_obj_t args)
{
	double a = 1.0;
	while (scm_is_pair(args)) {
		scm_obj_t arg = scm_car(args);
		if (!scm_is_number(arg)) return scm_error("*: needs a number");
		a *= scm_number_value(arg);
		args = scm_cdr(args);
	}
	return scm_number(a);
}

extern scm_obj_t scm_div(scm_obj_t args)
{
	if (!scm_is_pair(args)) return scm_error("/: needs an argument");
	scm_obj_t arg = scm_car(args);
	if (!scm_is_number(arg)) return scm_error("/: needs a number");
	double a = scm_number_value(arg);
	args = scm_cdr(args);
	if (scm_is_empty_list(args)) return scm_number(1.0/a);

	while (scm_is_pair(args)) {
		arg = scm_car(args);
		if (!scm_is_number(arg)) return scm_error("/: needs a number");
		double b = scm_number_value(arg);
		if (b == 0.0) return scm_error("/: division by zero");
		a /= b;
		args = scm_cdr(args);
	}
	return scm_number(a);
}

extern scm_obj_t scm_numeric_equal(scm_obj_t args)
{
	if (!scm_is_pair(args)) return scm_error("=: needs an argument");
	scm_obj_t arg = scm_car(args);
	if (!scm_is_number(arg)) return scm_error("=: needs a number");
	double a = scm_number_value(arg);
	args = scm_cdr(args);
	if (scm_is_empty_list(args)) return scm_true();

	while (scm_is_pair(args)) {
		arg = scm_car(args);
		if (!scm_is_number(arg)) return scm_error("=: needs a number");
		if (a != scm_number_value(arg)) return scm_false();
		args = scm_cdr(args);
	}
	return scm_true();
}

extern scm_obj_t scm_quotient(scm_obj_t args)
{
	double a, b;
	if (!scm_is_pair(args)) return scm_error("quotient: needs 2 numbers");
	scm_obj_t arg = scm_car(args);
	if (!scm_is_number(arg)) return scm_error("quotient: needs a number");
	a = scm_number_value(arg);
	args = scm_cdr(args);
	if (!scm_is_pair(args)) return scm_error("quotient: needs 2 numbers");
	arg = scm_car(args);
	if (!scm_is_number(arg)) return scm_error("quotient: needs a number");
	b = scm_number_value(arg);
	args = scm_cdr(args);
	if (!scm_is_empty_list(args)) return scm_error("quotient: needs 2 numbers");
	if (b == 0.0) return scm_error("quotient: division by zero");
	return scm_number(trunc(a / b));
}

extern scm_obj_t scm_modulo(scm_obj_t args)
{
	double a, b;
	if (!scm_is_pair(args)) return scm_error("modulo: needs 2 numbers");
	scm_obj_t arg = scm_car(args);
	if (!scm_is_number(arg)) return scm_error("modulo: needs a number");
	a = scm_number_value(arg);
	args = scm_cdr(args);
	if (!scm_is_pair(args)) return scm_error("modulo: needs 2 numbers");
	arg = scm_car(args);
	if (!scm_is_number(arg)) return scm_error("modulo: needs a number");
	b = scm_number_value(arg);
	args = scm_cdr(args);
	if (!scm_is_empty_list(args)) return scm_error("modulo: needs 2 numbers");
	if (b == 0.0) return scm_error("modulo: division by zero");
	return scm_number(a - b * floor(a / b));
}

extern scm_obj_t scm_apply(scm_obj_t proc, scm_obj_t args)
{
	uint32_t procedure;

	if (scm_is_procedure(proc)) {
		procedure = scm_procedure_id(proc);
		if (procedure == SCM_PROCEDURE_ADD) return scm_add(args);
		if (procedure == SCM_PROCEDURE_SUB) return scm_sub(args);
		if (procedure == SCM_PROCEDURE_MUL) return scm_mul(args);
		if (procedure == SCM_PROCEDURE_DIV) return scm_div(args);
		if (procedure == SCM_PROCEDURE_WRITE) return scm_write(args);
		if (procedure == SCM_PROCEDURE_CAR) return scm_car(scm_car(args));
		if (procedure == SCM_PROCEDURE_CDR) return scm_cdr(scm_car(args));
		if (procedure == SCM_PROCEDURE_IS_EQ) return scm_is_eq(scm_car(args), scm_car(scm_cdr(args)));
		if (procedure == SCM_PROCEDURE_IS_NULL) return scm_boolean(scm_is_empty_list(scm_car(args)));
		if (procedure == SCM_PROCEDURE_IS_BOOLEAN) return scm_boolean(scm_is_boolean(scm_car(args)));
		if (procedure == SCM_PROCEDURE_IS_EOF_OBJECT) return scm_boolean(scm_is_eof_object(scm_car(args)));
		if (procedure == SCM_PROCEDURE_IS_SYMBOL) return scm_boolean(scm_is_symbol(scm_car(args)));
		if (procedure == SCM_PROCEDURE_IS_STRING) return scm_boolean(scm_is_string(scm_car(args)));
		if (procedure == SCM_PROCEDURE_IS_PAIR) return scm_boolean(scm_is_pair(scm_car(args)));
		if (procedure == SCM_PROCEDURE_IS_CHAR) return scm_boolean(scm_is_char(scm_car(args)));
		if (procedure == SCM_PROCEDURE_IS_NUMBER) return scm_boolean(scm_is_number(scm_car(args)));
		if (procedure == SCM_PROCEDURE_CONS) return scm_cons(scm_car(args), scm_car(scm_cdr(args)));
		if (procedure == SCM_PROCEDURE_SET_CAR) return scm_set_car(scm_car(args), scm_car(scm_cdr(args)));
		if (procedure == SCM_PROCEDURE_SET_CDR) return scm_set_cdr(scm_car(args), scm_car(scm_cdr(args)));
		if (procedure == SCM_PROCEDURE_NUMERIC_EQUAL) return scm_numeric_equal(args);
		if (procedure == SCM_PROCEDURE_MODULO) return scm_modulo(args);
		if (procedure == SCM_PROCEDURE_QUOTIENT) return scm_quotient(args);
		else return scm_error("unknown procedure");
	}
	else if (scm_is_closure(proc)) {
		proc = (proc & ~SCM_MASK) | SCM_PAIR;
		scm_obj_t env = scm_car(proc);
		scm_obj_t params = scm_car(scm_cdr(proc));
		scm_obj_t body = scm_cdr(scm_cdr((proc)));

		/* extend environment: bind params to args */
		scm_obj_t new_environment = scm_environment_extend(env, params, args);
		if (scm_is_error_object(new_environment)) return new_environment;

		/* eval body expressions in order, return last */
		scm_obj_t result = scm_unspecified();
		while (scm_is_pair(body)) {
#ifdef DEBUG
			printf("; apply: calling (eval "); scm_write(scm_car(body)); printf(" "); scm_write(new_environment); printf(")\n");
#endif
			result = scm_eval(scm_car(body), new_environment);
			if (scm_is_error_object(result)) return result;
			body = scm_cdr(body);
		}
		return result;
	}
	else {
		return scm_error("apply: attempt to apply non-procedure");
	}
}
