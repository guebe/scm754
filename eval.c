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

static scm_obj_t eval_if(scm_obj_t args, scm_obj_t env)
{
	scm_obj_t cond = scm_car(args);
	args = scm_cdr(args);

	scm_obj_t then = scm_car(args);
	args = scm_cdr(args);

	scm_obj_t else_ = scm_nil();
	if (!scm_is_null(args)) {
		else_ = scm_car(args);
		if (!scm_is_null(scm_cdr(args))) return scm_error("if: bad form, should be (if expr then [else])");
	}
	cond = scm_eval(cond, env);
	if (scm_is_error(cond)) return cond;

	return scm_boolean_value(cond) ? then
				       : (!scm_is_null(else_) ? else_ : scm_unspecified());
}

static scm_obj_t eval_define(scm_obj_t args, scm_obj_t env)
{
	scm_obj_t value;
	scm_obj_t var = scm_car(args);
	args = scm_cdr(args);
	if (scm_is_symbol(var)) { /* variable define */
		if (!scm_is_null(scm_cdr(args))) return scm_error("define: bad form, should be (define x expr)");
		value = scm_eval(scm_car(args), env);
		goto out;
	}
	else if (scm_is_pair(var) && scm_is_symbol(scm_car(var))) { /* function define - de-sugar to lambda */
		if (scm_is_null(args)) return scm_error("define: bad form, should be (define (f x y) body...)");
		value = scm_eval(scm_cons(scm_lambda, scm_cons(scm_cdr(var), args)), env);
		var = scm_car(var);
		goto out;
	}
	else return scm_error("define: bad form, should be (define var value) or (define (f x y) body)");
out:
	if (scm_is_error(value)) return value;
	scm_environment_define(env, scm_intern(var), value);
	return scm_unspecified();
}

/* convert the lambda to a closure and capture the environment */
static scm_obj_t eval_lambda(scm_obj_t args, scm_obj_t env)
{
	return scm_closure(scm_cons(env, args));
}

static scm_obj_t eval_quote(scm_obj_t args)
{
	if (!scm_is_null(scm_cdr(args))) return scm_error("quote: bad form, should be (quote datum)");
	return scm_car(args);
}

static bool get_let_binding(scm_obj_t first, scm_obj_t *param, scm_obj_t *value)
{
	if (!scm_is_pair(first)) return false;
	scm_obj_t second = scm_cdr(first);
	if (!scm_is_pair(second) || !scm_is_null(scm_cdr(second))) return false;
	*param = scm_car(first);
	*value = scm_car(second);
	return true;
}

/* desugar (let ((x y)...)) body...) -> (lambda (xs...) body...) ys... */
static scm_obj_t eval_let(scm_obj_t args)
{
	scm_obj_t param_list = scm_nil(), value_list = scm_nil();
	scm_obj_t param_tail, value_tail, param, value;

	scm_obj_t bindings = scm_car(args);
	scm_obj_t body = scm_cdr(args);

	if (scm_is_null(body)) return scm_error("let: bad form, body missing");

	while (scm_is_pair(bindings)) {
		if (!get_let_binding(scm_car(bindings), &param, &value))
			return scm_error("let: bad form in binding");

		scm_obj_t param_pair = scm_cons(param, scm_nil());
		scm_obj_t value_pair = scm_cons(value, scm_nil());

		if (scm_is_null(param_list)) {
			param_list = param_tail = param_pair;
			value_list = value_tail = value_pair;
		}
		else {
			scm_set_cdr(param_tail, param_pair);
			scm_set_cdr(value_tail, value_pair);
			param_tail = param_pair;
			value_tail = value_pair;
		}

		bindings = scm_cdr(bindings);
	}
	if (!scm_is_null(bindings)) return scm_error("let: improper binding list");

	return scm_cons(scm_cons(scm_lambda, scm_cons(param_list, body)), value_list);
}

/* desugar (let* ((x y)...) body...) -> (lambda (x) (lambda... body...)) y */
static scm_obj_t eval_let_star(scm_obj_t args)
{
	scm_obj_t bindings = scm_car(args);
	scm_obj_t body = scm_cdr(args);

	if (scm_is_null(body)) return scm_error("let*: bad form, body missing");

	/* no binding ((lambda () body...)) */
	if (scm_is_null(bindings)) return scm_cons(scm_cons(scm_lambda, scm_cons(scm_nil(), body)), scm_nil());

	if (!scm_is_pair(bindings)) return scm_error("let*: bindings must be a list");

	/* first binding ((lambda (param) body...) value) */
	scm_obj_t param, value, prev, inner;
	if (!get_let_binding(scm_car(bindings), &param, &value))
		return scm_error("let*: bad form in binding");

	prev = inner = scm_cons(scm_cons(param, scm_nil()), body);
	scm_obj_t root = scm_cons(scm_cons(scm_lambda, inner), scm_cons(value, scm_nil()));

	/* other bindings ((lambda (param1) ((lambda (param2) body...) value2)) value1) */
	bindings = scm_cdr(bindings);
	while (scm_is_pair(bindings)) {
		if (!get_let_binding(scm_car(bindings), &param, &value))
			return scm_error("let*: bad form in binding");

		inner = scm_cons(scm_cons(param, scm_nil()), body);
		scm_obj_t outer = scm_cons(scm_cons(scm_lambda, inner), scm_cons(value, scm_nil()));
		scm_set_cdr(prev, scm_cons(outer, scm_nil()));
		prev = inner;

		bindings = scm_cdr(bindings);
	}
	if (!scm_is_null(bindings)) return scm_error("let*: improper binding list");

	return root;
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

		/* special forms */
		if (scm_is_symbol(op)) {
			if (op == scm_quote) return eval_quote(args);
			else if (op == scm_define) return eval_define(args, env);
			else if (op == scm_lambda) return eval_lambda(args, env);
			else if (op == scm_if) {
				expr = eval_if(args, env);
				if (scm_is_unspecified(expr) || scm_is_error(expr)) return expr;
				continue; /* tail call */
			}
			else if (op == scm_let) {
				expr = eval_let(args);
				if (scm_is_error(expr)) return expr;
				continue; /* tail call */
			}
			else if (op == scm_let_star) {
				expr = eval_let_star(args);
				if (scm_is_error(expr)) return expr;
				continue; /* tail call */
			}
		}

		/* application */
		op = scm_eval(op, env);
		if (scm_is_error(op)) return op;
		size_t argc = scm_length(args);
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
			op = scm_closure_value(op);
			env = scm_environment_extend(scm_car(op), scm_car(scm_cdr(op)), args);
			if (scm_is_error(env)) return env;
			free_list(args);

			/* eval body expressions in order */
			scm_obj_t body = scm_cdr(scm_cdr((op)));
			while (scm_is_pair(scm_cdr(body))) {
#ifdef DEBUG
printf("; apply: calling (eval "); scm_write(scm_car(body)); printf(" "); scm_write(new); printf(")\n");
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
	if (!scm_is_procedure(proc)) return scm_error("apply: attempt to apply non-procedure");

	uint32_t procedure = scm_procedure_id(proc);
	scm_obj_t arg1 = scm_car(args);
	scm_obj_t arg2 = scm_car(scm_cdr(args));

	switch (procedure) {
	case SCM_PROCEDURE_NEWLINE: return argc == 0 ? scm_newline() : scm_error("newline: takes no parameter");
	case SCM_PROCEDURE_CAR: return argc == 1 ? scm_car(arg1) : scm_error("car: takes one parameter");
	case SCM_PROCEDURE_CDR: return argc == 1 ? scm_cdr(arg1) : scm_error("cdr: takes one parameter");
	case SCM_PROCEDURE_IS_PROCEDURE: return argc == 1 ? scm_boolean(scm_is_procedure(arg1) || scm_is_closure(arg1)) : scm_error("procedure?: takes one parameter");
	case SCM_PROCEDURE_IS_NULL: return argc == 1 ? scm_boolean(scm_is_null(arg1)) : scm_error("null?: takes one parameter");
	case SCM_PROCEDURE_IS_BOOLEAN: return argc == 1 ? scm_boolean(scm_is_boolean(arg1)) : scm_error("boolean?: takes one parameter");
	case SCM_PROCEDURE_IS_EOF_OBJECT: return argc == 1 ? scm_boolean(scm_is_eof_object(arg1)) : scm_error("eof-object?: takes one parameter");
	case SCM_PROCEDURE_IS_SYMBOL: return argc == 1 ? scm_boolean(scm_is_symbol(arg1)) : scm_error("symbol?: takes one parameter");
	case SCM_PROCEDURE_IS_STRING: return argc == 1 ? scm_boolean(scm_is_string(arg1)) : scm_error("string?: takes one parameter");
	case SCM_PROCEDURE_IS_PAIR: return argc == 1 ? scm_boolean(scm_is_pair(arg1)) : scm_error("pair?: takes one parameter");
	case SCM_PROCEDURE_IS_CHAR: return argc == 1 ? scm_boolean(scm_is_char(arg1)) : scm_error("char?: takes one parameter");
	case SCM_PROCEDURE_IS_NUMBER: return argc == 1 ? scm_boolean(scm_is_number(arg1)) : scm_error("number?: takes one parameter");
	case SCM_PROCEDURE_LENGTH: return argc == 1 ? scm_number((double)scm_length(arg1)) : scm_error("length: takes one parameter");
	case SCM_PROCEDURE_DISPLAY: return argc == 1 ? scm_display(arg1) : scm_error("display: takes one parameter");
	case SCM_PROCEDURE_WRITE: return argc == 1 ? scm_write(arg1) : scm_error("write: takes one parameter");
	case SCM_PROCEDURE_LOAD: {
		if ((argc != 1) || !scm_is_string(arg1)) return scm_error("load: takes one string parameter");
		scm_obj_t tmp = scm_load(scm_string_value(arg1));
		return scm_is_error(tmp) ? tmp : scm_unspecified();
	}
	case SCM_PROCEDURE_IS_ZERO: return argc == 1 ? scm_is_zero(arg1) : scm_error("zero?: takes one parameter");
	case SCM_PROCEDURE_STRING_LENGTH: return argc == 1 ? scm_number((double)scm_string_length(arg1)) : scm_error("string-length: takes one parameter");
	case SCM_PROCEDURE_IS_EQ: return argc == 2 ? scm_is_eq(arg1, arg2) : scm_error("eq?: takes two parameter");
	case SCM_PROCEDURE_CONS: return argc == 2 ? scm_cons(arg1, arg2) : scm_error("cons: takes two parameter");
	case SCM_PROCEDURE_SET_CAR: return argc == 2 ? scm_set_car(arg1, arg2) : scm_error("set-car!: takes two parameter");
	case SCM_PROCEDURE_SET_CDR: return argc == 2 ? scm_set_cdr(arg1, arg2) : scm_error("set-cdr!: takes two parameter");
	case SCM_PROCEDURE_MODULO: return argc == 2 ? scm_modulo(arg1, arg2) : scm_error("modulo: takes two parameter");
	case SCM_PROCEDURE_QUOTIENT: return argc == 2 ? scm_quotient(arg1, arg2) : scm_error("quotient: takes two parameter");
	case SCM_PROCEDURE_ADD: return scm_add(args);
	case SCM_PROCEDURE_SUB: return scm_sub(args);
	case SCM_PROCEDURE_MUL: return scm_mul(args);
	case SCM_PROCEDURE_DIV: return scm_div(args);
	case SCM_PROCEDURE_NUMBER_EQ: return scm_number_eq(args);
	case SCM_PROCEDURE_LT: return scm_lt(args);
	case SCM_PROCEDURE_GT: return scm_gt(args);
	case SCM_PROCEDURE_LE: return scm_le(args);
	case SCM_PROCEDURE_GE: return scm_ge(args);
	case SCM_PROCEDURE_STRING_EQ: return scm_string_eq(args);
	case SCM_PROCEDURE_STRING_COPY: return scm_string_copy(args);
	default: return scm_error("unknown procedure");
	}
}
