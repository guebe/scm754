/* (c) guenter.ebermann@htl-hl.ac.at */

#undef DEBUG

#include "scm754.h"
#include <math.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#if 0
static scm_obj_t scm_environment_rebind(scm_obj_t env, scm_obj_t params, scm_obj_t args)
{
	scm_obj_t a, b;

	a = params;
	b = args;

	while (scm_is_pair(a) && scm_is_pair(b)) {
		scm_set_car(scm_car(env), scm_car(b));
		a = scm_cdr(a);
		b = scm_cdr(b);
		env = scm_cdr(env);
	}

	if (!scm_is_empty_list(a) || !(scm_is_empty_list(b)))
		return scm_error("tail call: argument count mismatch");

	return scm_unspecified();
}
#endif

static scm_obj_t eval_list(scm_obj_t list, scm_obj_t environment_specifier)
{
	scm_obj_t head = scm_empty_list();
	scm_obj_t tail = scm_empty_list();

	while (scm_is_pair(list)) {
		scm_obj_t val = scm_eval(scm_car(list), environment_specifier);
		if (scm_is_error_object(val)) return val;

#ifdef DEBUG
printf("alloc1\n");
#endif
		scm_obj_t cell = scm_cons(val, scm_empty_list());

		if (scm_is_empty_list(head)) {
			head = cell;
			tail = cell;
		} else {
			scm_set_cdr(tail, cell);
			tail = cell;
		}

		list = scm_cdr(list);
	}

	if (!scm_is_empty_list(list))
		return scm_error("eval_list: improper list");

	return head;
}

extern scm_obj_t scm_eval(scm_obj_t expr_or_def, scm_obj_t environment_specifier)
{
	scm_obj_t expr = expr_or_def;
	scm_obj_t env  = environment_specifier;

	for (;;) {
		scm_obj_t op, args, evaled_args;

		if (scm_is_empty_list(expr)) return scm_error("eval: can not apply empty-list-object ()");

		if (scm_is_symbol(expr)) return scm_environment_lookup(env, scm_intern(expr)); /* symbols are bound to values or procedures in the environment */

		if (!scm_is_pair(expr)) return expr; /* self-evaluating */

		op = scm_car(expr);
		args = scm_cdr(expr);

		if (scm_is_symbol(op)) {
			/* special forms */
			if (op == scm_if) {
				if (!scm_is_pair(args)) return scm_error("eval: error in if form");
				scm_obj_t cond = scm_eval(scm_car(args), env);
				args = scm_cdr(args);
				if (!scm_is_pair(args)) return scm_error("eval: error in if form");
				scm_obj_t then = scm_car(args);
				scm_obj_t else_ = scm_cdr(args);
				if (scm_boolean_value(cond)) {
					expr = then;
					continue;
				}
				else if (!scm_is_empty_list(else_)) {
					expr = scm_car(else_);
					continue;
				}
				else return scm_unspecified();
			}
			if (op == scm_quote) {
				return scm_is_empty_list(scm_cdr(args)) ? scm_car(args) : scm_error("eval: bad quote form");
			}
			if (op == scm_define) {
				scm_obj_t var = scm_car(args);

				/* de-sugar define special form to lambda */
				if (scm_is_pair(var) && scm_is_symbol(scm_car(var))) {
					scm_obj_t name = scm_car(var);
					scm_obj_t params = scm_cdr(var);
					scm_obj_t rest = scm_cdr(args);
#ifdef DEBUG
					printf("alloc2\n");
#endif
					scm_obj_t lambda = scm_cons(scm_lambda, scm_cons(params, rest));
					return scm_eval(scm_cons(scm_define, scm_cons(name, scm_cons(lambda, scm_empty_list()))), env);
				}

				if (!scm_is_symbol(var)) return scm_error("eval: bad define form: variable is not a symbol");
				scm_obj_t tmp = scm_cdr(args);
				if (!scm_is_empty_list(scm_cdr(tmp))) return scm_error("eval: bad define form: too many arguments");

				scm_obj_t val = scm_eval(scm_car(tmp), env);
				if (scm_is_error_object(val)) return val;
				scm_environment_define(env, scm_intern(var), val);
				return scm_unspecified();
			}
			if (op == scm_lambda) {
				scm_obj_t params = scm_car(args); /* (a b) */
				scm_obj_t body = scm_cdr(args); /* ((+ a b)) */
#ifdef DEBUG
				printf("alloc3\n");
#endif
				return scm_closure((uint32_t)scm_cons(params, scm_cons(body, scm_cons(env, scm_empty_list())))); /* create a list of (params body environment) */
			}
		}

		/* application */
		scm_obj_t proc = scm_eval(op, env);
		if (scm_is_error_object(proc)) return proc;
		evaled_args = eval_list(args, env);
		if (scm_is_error_object(evaled_args)) return evaled_args;
#ifdef DEBUG
		printf("; eval: calling (apply "); scm_write(proc); printf(" "); scm_write(evaled_args); printf(")\n");
#endif
		if (scm_is_procedure(proc)) return scm_apply(proc, evaled_args);

		if (scm_is_closure(proc)) {
			scm_obj_t clos = (proc & ~SCM_MASK) | SCM_PAIR;
			scm_obj_t params = scm_car(clos);
			scm_obj_t body = scm_car(scm_cdr(clos));
			scm_obj_t cenv = scm_car(scm_cdr(scm_cdr((clos))));

			/* allocate new environment frame */
			cenv = scm_environment_extend(cenv, params, evaled_args);
			if (scm_is_error_object(cenv)) return cenv;

			/* eval body expressions in order, but last */
			while (scm_is_pair(scm_cdr(body))) {
#ifdef DEBUG
printf("; apply: (eval "	); scm_write(scm_car(body)); printf(")\n");
#endif
				scm_obj_t tmp = scm_eval(scm_car(body), cenv);
				if (scm_is_error_object(tmp)) return tmp;
				body = scm_cdr(body);
			}

			expr = scm_car(body);
			env = cenv;
			continue;
		}

		return scm_error("eval: not callable");
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
	else {
#ifdef DEBUG
		printf("; error: apply: attempt to apply non-procedure: "); scm_write(proc); printf("\n");
#endif
		return scm_error("apply: attempt to apply non-procedure");
	}
}
