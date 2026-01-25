/* (c) guenter.ebermann@htl-hl.ac.at */
#undef DEBUG
#include "scm754.h"

static scm_obj_t eval_list(scm_obj_t list, scm_obj_t environment_specifier)
{
	scm_obj_t head = scm_nil();
	scm_obj_t tail = scm_nil();

	while (scm_is_pair(list)) {
		scm_obj_t val = scm_eval(scm_car(list), environment_specifier);
		if (scm_is_error(val)) return val;

		scm_obj_t cell = scm_cons(val, scm_nil());
		if (scm_is_error(cell)) return cell;

		if (scm_is_null(head)) {
			head = cell;
			tail = cell;
		} else {
			scm_set_cdr(tail, cell);
			tail = cell;
		}

		list = scm_cdr(list);
	}

	if (!scm_is_null(list))
		return scm_error("eval_list: improper list");

	return head;
}

/* free value list: the values got copied into environment bindings by apply */
static void free_list(scm_obj_t evaled_args)
{
	while (scm_is_pair(evaled_args)) {
		scm_obj_t next = scm_cdr(evaled_args);
		scm_gc_free(evaled_args);
		evaled_args = next;
	}
}

static scm_obj_t eval_if(size_t argc, scm_obj_t args, scm_obj_t env)
{
	if (argc != 2 && argc != 3) return scm_error("if: bad form, should be (if expr then [else])");
	scm_obj_t cond = scm_eval(scm_car(args), env);
	if (scm_is_error(cond)) return cond;
	args = scm_cdr(args);
	if (scm_boolean_value(cond)) return scm_car(args);
	else if (argc == 3) return scm_car(scm_cdr(args));
	else return scm_unspecified();
}

static scm_obj_t eval_define(size_t argc, scm_obj_t args, scm_obj_t env)
{
	scm_obj_t value;
	scm_obj_t var = scm_car(args);
	if (scm_is_symbol(var)) { /* variable define */
		if (argc != 2) return scm_error("define: bad form, should be (define x expr)");
		value = scm_eval(scm_car(scm_cdr(args)), env);
		goto out;
	}
	else if (scm_is_pair(var) && scm_is_symbol(scm_car(var))) { /* function define - de-sugar to lambda */
		if (argc < 2) return scm_error("define: bad form, should be (define (f x y) body...)");
		value = scm_eval(scm_cons(scm_lambda, scm_cons(scm_cdr(var), scm_cdr(args))), env);
		var = scm_car(var);
		goto out;
	}
	else return scm_error("define: bad form, should be (define var value) or (define (f x y) body)");
out:
	if (scm_is_error(value)) return value;
	scm_environment_define(env, scm_intern(var), value);
	return scm_unspecified();
}

/* convert the lambda to a closure for later application: capture the environment at lamda definition time */
static scm_obj_t eval_lambda(size_t argc, scm_obj_t args, scm_obj_t env)
{
	if (argc < 2) return scm_error("lambda: bad form, should be (lambda (params...) body...");
	scm_obj_t param = scm_car(args);
	if (!scm_is_pair(param) && !scm_is_null(param) && !scm_is_symbol(param)) return scm_error("lambda: param must be a list, null or a symbol");
	return scm_closure((uint32_t)scm_cons(env, args));
}

static scm_obj_t eval_quote(size_t argc, scm_obj_t args)
{
	return argc == 1 ? scm_car(args) : scm_error("quote: bad form, should be (quote list)");
}

extern scm_obj_t scm_eval(scm_obj_t expr, scm_obj_t env)
{
	while (1) {
		if (scm_is_null(expr)) return scm_error("eval: can not apply empty list object ()");

		/* symbols are bound to values or procedures in the environment */
		else if (scm_is_symbol(expr)) return scm_environment_lookup(env, scm_intern(expr));

		/* self-evaluating */
		else if (!scm_is_pair(expr)) return expr;

		scm_obj_t op = scm_car(expr);
		scm_obj_t args = scm_cdr(expr);
		size_t argc = scm_length(args);

		/* special forms */
		if (scm_is_symbol(op)) {
			if (op == scm_quote) return eval_quote(argc, args);
			else if (op == scm_define) return eval_define(argc, args, env);
			else if (op == scm_lambda) return eval_lambda(argc, args, env);
			else if (op == scm_if) {
				expr = eval_if(argc, args, env);
				if (scm_is_unspecified(expr) || scm_is_error(expr)) return expr;
				continue; /* tail call */
			}
		}

		/* application */
		op = scm_eval(op, env);
		if (scm_is_error(op)) return op;
		args = eval_list(args, env);
		if (scm_is_error(args)) return args;
#ifdef DEBUG
		printf("; eval: calling (apply "); scm_write(op); printf(" "); scm_write(args); printf(")\n");
#endif
		if (scm_is_procedure(op)) {
			op = scm_apply(op, args, argc);
			free_list(args);
			return op;
		}
		else if (scm_is_closure(op)) {
			op = (op & ~SCM_MASK) | SCM_PAIR;
			env = scm_environment_extend(scm_car(op), scm_car(scm_cdr(op)), args);
			if (scm_is_error(env)) return env;
			free_list(args);

			/* eval body expressions in order */
			scm_obj_t body = scm_cdr(scm_cdr((op)));
			while (scm_is_pair(scm_cdr(body))) {
#ifdef DEBUG
printf("; apply: calling (eval "); scm_write(scm_car(body)); printf(" "); scm_write(new_env); printf(")\n");
#endif
				op = scm_eval(scm_car(body), env);
				if (scm_is_error(op)) return op;
				body = scm_cdr(body);
			}
			expr = scm_car(body);
			continue; /* tail call */
		}
		else return scm_error("eval: not callable");
	}
}

extern scm_obj_t scm_apply(scm_obj_t proc, scm_obj_t args, size_t argc)
{
	if (scm_is_procedure(proc)) {
		uint32_t procedure = scm_procedure_id(proc);
		scm_obj_t arg1 = scm_car(args);
		scm_obj_t arg2 = scm_car(scm_cdr(args));
		if (procedure == SCM_PROCEDURE_NEWLINE) return argc == 0 ? scm_newline() : scm_error("newline: takes no parameter");
		if (procedure == SCM_PROCEDURE_CAR) return argc == 1 ? scm_car(arg1) : scm_error("car: takes one parameter");
		else if (procedure == SCM_PROCEDURE_CDR) return argc == 1 ? scm_cdr(arg1) : scm_error("cdr: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_PROCEDURE) return argc == 1 ? scm_boolean(scm_is_procedure(arg1) || scm_is_closure(arg1)) : scm_error("procedure?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_NULL) return argc == 1 ? scm_boolean(scm_is_null(arg1)) : scm_error("null?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_BOOLEAN) return argc == 1 ? scm_boolean(scm_is_boolean(arg1)) : scm_error("boolean?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_EOF_OBJECT) return argc == 1 ? scm_boolean(scm_is_eof_object(arg1)) : scm_error("eof-object?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_SYMBOL) return argc == 1 ? scm_boolean(scm_is_symbol(arg1)) : scm_error("symbol?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_STRING) return argc == 1 ? scm_boolean(scm_is_string(arg1)) : scm_error("string?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_PAIR) return argc == 1 ? scm_boolean(scm_is_pair(arg1)) : scm_error("pair?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_CHAR) return argc == 1 ? scm_boolean(scm_is_char(arg1)) : scm_error("char?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_IS_NUMBER) return argc == 1 ? scm_boolean(scm_is_number(arg1)) : scm_error("number?: takes one parameter");
		else if (procedure == SCM_PROCEDURE_LENGTH) return argc == 1 ? scm_number((double)scm_length(arg1)) : scm_error("length: takes one parameter");
		else if (procedure == SCM_PROCEDURE_DISPLAY) return argc == 1 ? scm_display(arg1) : scm_error("display: takes one parameter");
		else if (procedure == SCM_PROCEDURE_WRITE) return argc == 1 ? scm_write(arg1) : scm_error("write: takes one parameter");
		else if (procedure == SCM_PROCEDURE_LOAD) { scm_obj_t tmp; return argc == 1 && scm_is_string(arg1) ? ((scm_is_error(tmp = scm_load(scm_string_value(arg1)))) ? tmp : scm_unspecified()) : scm_error("load: takes one string parameter"); }
		else if (procedure == SCM_PROCEDURE_IS_ZERO) return argc == 1 ? scm_is_zero(arg1) : scm_error("zero?: takes one parameter");
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
		else if (procedure == SCM_PROCEDURE_NUMERIC_EQUAL) return scm_numeric_equal(args);
		else if (procedure == SCM_PROCEDURE_LT) return scm_lt(args);
		else if (procedure == SCM_PROCEDURE_GT) return scm_gt(args);
		else if (procedure == SCM_PROCEDURE_LE) return scm_le(args);
		else if (procedure == SCM_PROCEDURE_GE) return scm_ge(args);
		else return scm_error("unknown procedure");
	}
	else {
		return scm_error("apply: attempt to apply non-procedure");
	}
}
