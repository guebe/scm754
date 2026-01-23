/* (c) guenter.ebermann@htl-hl.ac.at */

#undef DEBUG

#include "scm754.h"
#ifdef DEBUG
#include <stdio.h>
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

extern scm_obj_t scm_eval(scm_obj_t expr, scm_obj_t env)
{
	while (1) {
		if (scm_is_empty_list(expr)) return scm_error("eval: can not apply empty-list-object ()");

		/* symbols are bound to values or procedures in the environment */
		else if (scm_is_symbol(expr)) return scm_environment_lookup(env, scm_intern(expr));

		/* self-evaluating */
		else if (!scm_is_pair(expr)) return expr;

		scm_obj_t op = scm_car(expr);
		scm_obj_t args = scm_cdr(expr);
		size_t argc = scm_length(args);

		/* special forms */
		if (scm_is_symbol(op)) {
			if (op == scm_if) {
				if (argc != 2 && argc != 3) return scm_error("if: bad form, should be (if expr then [else])");
				if (scm_boolean_value(scm_eval(scm_car(args), env))) {
					expr = scm_car(scm_cdr(args)); /* then */
					continue;
				}
				else if (argc == 3) {
					expr = scm_car(scm_cdr(scm_cdr(args))); /* else */
					continue;
				}
				else return scm_unspecified();
			}
			else if (op == scm_quote) {
				return argc == 1 ? scm_car(args) : scm_error("quote: bad form, should be (quote list)");
			}
			else if (op == scm_define) {
				scm_obj_t var = scm_car(args);
				if (scm_is_symbol(var)) {
					/* variable define */
					if (argc != 2) return scm_error("define: bad form, should be (define x expr)");
					scm_obj_t value = scm_eval(scm_car(scm_cdr(args)), env);
					if (scm_is_error_object(value)) return value;
					scm_environment_define(env, scm_intern(var), value);
					return scm_unspecified();
				}
				else if (scm_is_pair(var) && scm_is_symbol(scm_car(var))) {
					/* function define - de-sugar to lambda */
					if (argc < 2) return scm_error("define: bad form, should be (define (f x y) body...)");
					scm_obj_t value = scm_eval(scm_cons(scm_lambda, scm_cons(scm_cdr(var), scm_cdr(args))), env);
					if (scm_is_error_object(value)) return value;
					scm_environment_define(env, scm_intern(scm_car(var)), value);
					return scm_unspecified();
				}
				else return scm_error("define: neither a variable not function define");
			}
			else if (op == scm_lambda) {
				if (argc < 2) return scm_error("lamdba: bad form, should be (lambda (params...) body...");
				scm_obj_t param = scm_car(args);
				if (!scm_is_pair(param) && !scm_is_empty_list(param) && !scm_is_symbol(param)) return scm_error("lambda: param must be a list, null or a symbol");
				/* convert the lambda to a closure for later application: capture the environment at lamda definition time */
				return scm_closure((uint32_t)scm_cons(env, args));
			}
		}

		/* application */
		scm_obj_t proc = scm_eval(op, env);
		if (scm_is_error_object(proc)) return proc;
		scm_obj_t evaled_args = eval_list(args, env);
		if (scm_is_error_object(evaled_args)) return evaled_args;
#ifdef DEBUG
		printf("; eval: calling (apply "); scm_write(proc); printf(" "); scm_write(evaled_args); printf(")\n");
#endif
		return scm_apply(proc, evaled_args, argc);
	}
}

extern scm_obj_t scm_apply(scm_obj_t proc, scm_obj_t args, size_t argc)
{
	if (scm_is_procedure(proc)) {
		uint32_t procedure = scm_procedure_id(proc);
		scm_obj_t arg1 = scm_car(args);
		scm_obj_t arg2 = scm_car(scm_cdr(args));
		if (procedure == SCM_PROCEDURE_CAR) return argc == 1 ? scm_car(arg1) : scm_error("car: takes one parameter");
		else if (procedure == SCM_PROCEDURE_CDR) return argc == 1 ? scm_cdr(arg1) : scm_error("cdr: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_PROCEDURE) return argc == 1 ? scm_boolean(scm_is_procedure(arg1) || scm_is_closure(arg1)) : scm_error("procedure?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_NULL) return argc == 1 ? scm_boolean(scm_is_empty_list(arg1)) : scm_error("null?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_BOOLEAN) return argc == 1 ? scm_boolean(scm_is_boolean(arg1)) : scm_error("boolean?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_EOF_OBJECT) return argc == 1 ? scm_boolean(scm_is_eof_object(arg1)) : scm_error("eof-object?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_SYMBOL) return argc == 1 ? scm_boolean(scm_is_symbol(arg1)) : scm_error("symbol?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_STRING) return argc == 1 ? scm_boolean(scm_is_string(arg1)) : scm_error("string?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_PAIR) return argc == 1 ? scm_boolean(scm_is_pair(arg1)) : scm_error("pair?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_CHAR) return argc == 1 ? scm_boolean(scm_is_char(arg1)) : scm_error("char?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_NUMBER) return argc == 1 ? scm_boolean(scm_is_number(arg1)) : scm_error("number?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_LENGTH) return argc == 1 ? scm_number((double)scm_length(arg1)) : scm_error("length: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_EQ) return argc == 2 ? scm_is_eq(arg1, arg2) : scm_error("eq?: takes two parameter");
		else if (procedure == SCM_PROCEDURE_CONS) return argc == 2 ? scm_cons(arg1, arg2) : scm_error("cons: takes two parameter");
		else if (procedure == SCM_PROCEDURE_SET_CAR) return argc == 2 ? scm_set_car(arg1, arg2) : scm_error("set-car!: takes two parameter");
		else if (procedure == SCM_PROCEDURE_SET_CDR) return argc == 2 ? scm_set_cdr(arg1, arg2) : scm_error("set-cdr!: takes two parameter");
		else if (procedure == SCM_PROCEDURE_MODULO) return argc == 2 ? scm_modulo(arg1, arg2) : scm_error("modulo: takes two parameter");
		else if (procedure == SCM_PROCEDURE_QUOTIENT) return argc == 2 ? scm_quotient(arg1, arg2) : scm_error("quotient: takes two parameter");
		else if (procedure == SCM_PROCEDURE_ADD) return scm_add(args);
		else if (procedure == SCM_PROCEDURE_SUB) return scm_sub(args);
		else if (procedure == SCM_PROCEDURE_MUL) return scm_mul(args);
		else if (procedure == SCM_PROCEDURE_DIV) return scm_div(args);
		else if (procedure == SCM_PROCEDURE_WRITE) return scm_write(args);
		else if (procedure == SCM_PROCEDURE_NUMERIC_EQUAL) return scm_numeric_equal(args);
		else return scm_error("unknown procedure");
	}
	else if (scm_is_closure(proc)) {
		proc = (proc & ~SCM_MASK) | SCM_PAIR;
		scm_obj_t env = scm_car(proc);
		scm_obj_t params = scm_car(scm_cdr(proc));
		scm_obj_t body = scm_cdr(scm_cdr((proc)));

		/* extend environment: bind params to args */
		scm_obj_t new_env = scm_environment_extend(env, params, args);
		if (scm_is_error_object(new_env)) return new_env;

		/* eval body expressions in order, return last */
		scm_obj_t result = scm_unspecified();
		while (scm_is_pair(body)) {
#ifdef DEBUG
			printf("; apply: calling (eval "); scm_write(scm_car(body)); printf(" "); scm_write(new_environment); printf(")\n");
#endif
			result = scm_eval(scm_car(body), new_env);
			if (scm_is_error_object(result)) return result;
			body = scm_cdr(body);
		}
		return result;
	}
	else {
		return scm_error("apply: attempt to apply non-procedure");
	}
}
