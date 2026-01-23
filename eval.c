/* (c) guenter.ebermann@htl-hl.ac.at */

#undef DEBUG

#include "scm754.h"
#ifdef DEBUG
#include <stdio.h>
#endif

static scm_obj_t eval_list(scm_obj_t list, scm_obj_t environment_specifier, scm_obj_t argv[])
{
	size_t i = 0;
	while (scm_is_pair(list)) {
		scm_obj_t value = scm_eval(scm_car(list), environment_specifier);
		if (scm_is_error_object(value)) return value;
		argv[i] = value;
		i++;
		list = scm_cdr(list);
	}
	return scm_unspecified();
}

extern scm_obj_t scm_eval(scm_obj_t expr_or_def, scm_obj_t environment_specifier)
{
	scm_obj_t op, args;

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
#ifdef DEBUG
printf("alloc1: 2\n");
#endif
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
#ifdef DEBUG
printf("alloc2: 1\n");
#endif
				return scm_closure((uint32_t)scm_cons(environment_specifier, args));
			}
		}

		/* application */
		scm_obj_t proc = scm_eval(op, environment_specifier);
		if (scm_is_error_object(proc)) return proc;
		size_t argc = scm_length(args);
		scm_obj_t argv[argc > 0 ? argc : 1];
		scm_obj_t ret = eval_list(args, environment_specifier, argv);
		if (scm_is_error_object(ret)) return ret;
#ifdef DEBUG
		printf("; eval: calling (apply "); scm_write(proc); printf(")\n");
#endif
		return scm_apply(proc, argv, argc);
	}
	else {
		/* self-evaluating */
		return expr_or_def;
	}
}

extern scm_obj_t scm_apply(scm_obj_t proc, scm_obj_t argv[], size_t argc)
{
	uint32_t procedure;

	if (scm_is_procedure(proc)) {
		procedure = scm_procedure_id(proc);
		if (procedure == SCM_PROCEDURE_ADD) return scm_add(argv, argc);
		if (procedure == SCM_PROCEDURE_SUB) return scm_sub(argv, argc);
		if (procedure == SCM_PROCEDURE_MUL) return scm_mul(argv, argc);
		if (procedure == SCM_PROCEDURE_DIV) return scm_div(argv, argc);
		if (procedure == SCM_PROCEDURE_CAR) return argc == 1 ? scm_car(argv[0]) : scm_error("apply: car: takes one parameter");
		if (procedure == SCM_PROCEDURE_CDR) return argc == 1 ? scm_cdr(argv[0]) : scm_error("apply: cdr: takes one parameter");
		if (procedure == SCM_PROCEDURE_IS_EQ) return argc == 2 ? scm_is_eq(argv[0], argv[1]) : scm_error("apply: eq?: takes two parameter");
		if (procedure == SCM_PROCEDURE_IS_NULL) return argc == 1 ? scm_boolean(scm_is_empty_list(argv[0])) : scm_error("apply: null?: takes one parameter");
		if (procedure == SCM_PROCEDURE_IS_BOOLEAN) return argc == 1 ? scm_boolean(scm_is_boolean(argv[0])) : scm_error("apply: boolean?: takes one parameter");
		if (procedure == SCM_PROCEDURE_IS_EOF_OBJECT) return argc == 1 ? scm_boolean(scm_is_eof_object(argv[0])) : scm_error("apply: eof-object?: takes one parameter");
		if (procedure == SCM_PROCEDURE_IS_SYMBOL) return argc == 1 ? scm_boolean(scm_is_symbol(argv[0])) : scm_error("apply: symbol?: takes one parameter");
		if (procedure == SCM_PROCEDURE_IS_STRING) return argc == 1 ? scm_boolean(scm_is_string(argv[0])) : scm_error("apply: string?: takes one parameter");
		if (procedure == SCM_PROCEDURE_IS_PAIR) return argc == 1 ? scm_boolean(scm_is_pair(argv[0])) : scm_error("apply: pair?: takes one parameter");
		if (procedure == SCM_PROCEDURE_IS_CHAR) return argc == 1 ? scm_boolean(scm_is_char(argv[0])) : scm_error("apply: char?: takes one parameter");
		if (procedure == SCM_PROCEDURE_IS_NUMBER) return argc == 1 ? scm_boolean(scm_is_number(argv[0])) : scm_error("apply: number?: takes one parameter");
		if (procedure == SCM_PROCEDURE_CONS) return argc == 2 ? scm_cons(argv[0], argv[1]) : scm_error("apply: cons: takes two parameter");
		if (procedure == SCM_PROCEDURE_SET_CAR) return argc == 2 ? scm_set_car(argv[0], argv[1]) : scm_error("apply: set-car!: takes two parameter");
		if (procedure == SCM_PROCEDURE_SET_CDR) return argc == 2 ? scm_set_cdr(argv[0], argv[1]) : scm_error("apply: set-cdr!: takes two parameter");
		if (procedure == SCM_PROCEDURE_NUMERIC_EQUAL) return scm_numeric_equal(argv, argc);
		if (procedure == SCM_PROCEDURE_MODULO) return argc == 2 ? scm_modulo(argv[0], argv[1]) : scm_error("apply: modulo: takes two parameter");
		if (procedure == SCM_PROCEDURE_QUOTIENT) return argc == 2 ? scm_quotient(argv[0], argv[1]) : scm_error("apply: quotient: takes two parameter");
		else return scm_error("unknown procedure");
	}
	else if (scm_is_closure(proc)) {
		proc = (proc & ~SCM_MASK) | SCM_PAIR;
		scm_obj_t env = scm_car(proc);
		scm_obj_t params = scm_car(scm_cdr(proc));
		scm_obj_t body = scm_cdr(scm_cdr((proc)));

		/* extend environment: bind params to argv, argc */
#ifdef DEBUG
printf("alloc3: %lu\n", 2*argc + 1);
#endif
		scm_obj_t new_environment = scm_environment_extend(env, params, argv, argc);
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
