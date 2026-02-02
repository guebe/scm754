/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

static bool debug = false;

static scm_obj_t eval_list(scm_obj_t list, scm_obj_t environment_specifier)
{
	scm_obj_t head = scm_nil();
	scm_obj_t tail = scm_nil();
	scm_obj_t result;

	if (!scm_gc_push(&head)) return scm_error("eval: stack overflow");

	while (scm_is_pair(list)) {
		result = scm_eval(scm_car(list), environment_specifier);
		if (scm_is_error(result)) goto out;

		result = scm_cons(result, scm_nil());
		if (scm_is_error(result)) goto out;

		if (scm_is_null(head)) {
			head = tail = result;
		} else {
			scm_set_cdr(tail, result);
			tail = result;
		}

		list = scm_cdr(list);
	}

	result = head;

	if (!scm_is_null(list))
		result = scm_error("eval_list: improper list");

out:
	scm_gc_pop();
	return result;
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
		value = scm_eval(scm_cons(SCM_LAMBDA, scm_cons(scm_cdr(var), args)), env);
		var = scm_car(var);
		goto out;
	}
	else return scm_error("define: bad form, should be (define var value) or (define (f x y) body)");
out:
	if (scm_is_error(value)) return value;
	scm_environment_define(env, var, value);
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

	return scm_cons(scm_cons(SCM_LAMBDA, scm_cons(param_list, body)), value_list);
}

/* desugar (let* ((x y)...) body...) -> (lambda (x) (lambda... body...)) y */
static scm_obj_t eval_let_star(scm_obj_t args)
{
	scm_obj_t bindings = scm_car(args);
	scm_obj_t body = scm_cdr(args);

	if (scm_is_null(body)) return scm_error("let*: bad form, body missing");

	/* no binding ((lambda () body...)) */
	if (scm_is_null(bindings)) return scm_cons(scm_cons(SCM_LAMBDA, scm_cons(scm_nil(), body)), scm_nil());

	if (!scm_is_pair(bindings)) return scm_error("let*: bindings must be a list");

	/* first binding ((lambda (param) body...) value) */
	scm_obj_t param, value, prev, inner;
	if (!get_let_binding(scm_car(bindings), &param, &value))
		return scm_error("let*: bad form in binding");

	prev = inner = scm_cons(scm_cons(param, scm_nil()), body);
	scm_obj_t root = scm_cons(scm_cons(SCM_LAMBDA, inner), scm_cons(value, scm_nil()));

	/* other bindings ((lambda (param1) ((lambda (param2) body...) value2)) value1) */
	bindings = scm_cdr(bindings);
	while (scm_is_pair(bindings)) {
		if (!get_let_binding(scm_car(bindings), &param, &value))
			return scm_error("let*: bad form in binding");

		inner = scm_cons(scm_cons(param, scm_nil()), body);
		scm_obj_t outer = scm_cons(scm_cons(SCM_LAMBDA, inner), scm_cons(value, scm_nil()));
		scm_set_cdr(prev, scm_cons(outer, scm_nil()));
		prev = inner;

		bindings = scm_cdr(bindings);
	}
	if (!scm_is_null(bindings)) return scm_error("let*: improper binding list");

	return root;
}

static scm_obj_t eval_and(scm_obj_t args, scm_obj_t env)
{
	if (scm_is_null(args)) return scm_true();

	scm_obj_t test;
	while (scm_is_pair(scm_cdr(args))) {
		test = scm_eval(scm_car(args), env);
		if (!scm_boolean_value(test)) return scm_false();
		args = scm_cdr(args);
	}

	return scm_car(args);
}

static scm_obj_t eval_or(scm_obj_t args, scm_obj_t env)
{
	if (scm_is_null(args)) return scm_false();

	scm_obj_t test;
	while (scm_is_pair(scm_cdr(args))) {
		test = scm_eval(scm_car(args), env);
		if (scm_boolean_value(test)) return test;
		args = scm_cdr(args);
	}

	return scm_car(args);
}

extern void scm_enable_debug(void)
{
	debug = true;
}

static void debug_print(bool is_closure, scm_obj_t op, scm_obj_t args)
{
	if (is_closure) fputs("; lambda ", stdout);
	else fputs("; ", stdout);
	scm_write(op);
	putchar(' ');
	scm_write(args);
	putchar('\n');
}

static scm_obj_t apply_closure(scm_obj_t proc, scm_obj_t evlist, scm_obj_t env)
{
	if (debug) debug_print(true, proc, evlist);

	scm_obj_t body = scm_cdr(proc);
	while (scm_is_pair(scm_cdr(body))) {
	    scm_obj_t result = scm_eval(scm_car(body), env);
	    if (scm_is_error(result)) return result;
	    body = scm_cdr(body);
	}
	return scm_car(body);
}

extern scm_obj_t scm_eval(scm_obj_t expr, scm_obj_t env)
{
	scm_obj_t result;

	if (!scm_gc_push2(&expr, &env)) {
		return scm_error("eval: stack overflow");
	}

tail_call:

	if (scm_is_null(expr)) {
		result = scm_error("eval: can not eval empty list object ()");
	}
	else if (scm_is_symbol(expr)) {
		result = scm_environment_lookup(env, expr);
	}
	else if (!scm_is_pair(expr)) {
		result = expr; /* self-evaluating */
	}
	else {
		scm_obj_t op = scm_car(expr);
		scm_obj_t args = scm_cdr(expr);

		/* special forms */
		if (scm_is_symbol(op)) {
			switch (scm_procedure_id(op)) {
			case SCM_OP_QUOTE:
				result = eval_quote(args);
				goto out;
			case SCM_OP_DEFINE:
				result = eval_define(args, env);
				goto out;
			case SCM_OP_LAMBDA:
				result = eval_lambda(args, env);
				goto out;
			case SCM_OP_IF:
				expr = result = eval_if(args, env);
				if (scm_is_unspecified(result) || scm_is_error(result)) goto out; else goto tail_call;
			case SCM_OP_LET:
				expr = result = eval_let(args);
				if (scm_is_error(result)) goto out; else goto tail_call;
			case SCM_OP_LET_STAR:
				expr = result = eval_let_star(args);
				if (scm_is_error(result)) goto out; else goto tail_call;
			case SCM_OP_AND:
				expr = result = eval_and(args, env);
				if (scm_is_error(result)) goto out; else goto tail_call;
			case SCM_OP_OR:
				expr = result = eval_or(args, env);
				if (scm_is_error(result)) goto out; else goto tail_call;
			default: break;
			}
		}

		scm_gc_collect();

		if (!scm_gc_push2(&op, &args)) {
			return scm_error("eval: stack overflow");
		}

		/* application */
		op = result = scm_eval(op, env);
		if (scm_is_error(result)) goto out2;

		args = result = eval_list(args, env);
		if (scm_is_error(result)) goto out2;

		if (scm_is_procedure(op)) {
			result = scm_apply(op, args);
		}
		else if (scm_is_closure(op)) {
			op = scm_closure_value(op);
			scm_obj_t param_body = scm_cdr(op);
			env = result = scm_environment_extend(scm_car(op), scm_car(param_body), args);
			if (scm_is_error(result)) goto out2;
			expr = apply_closure(param_body, args, env);
			scm_gc_pop2();
			goto tail_call;
		}
		else {
			result = scm_error("eval: unknown expression type");
		}
	out2:
		scm_gc_pop2();
	}
out:
	scm_gc_pop2();
	return result;
}

extern scm_obj_t scm_apply(scm_obj_t proc, scm_obj_t args)
{
	if (!scm_is_procedure(proc)) return scm_error("apply: attempt to apply non-procedure");

	if (debug) debug_print(false, proc, args);

	int8_t arity = scm_procedure_arity(proc);
	size_t argc = scm_length(args);

	if (arity >= 0 && (size_t)arity != argc)
		return scm_error("%s: takes %d arguments, %lu given",
				 scm_procedure_string(proc), arity, argc);

	scm_obj_t arg1 = (argc >= 1) ? scm_car(args) : scm_nil();
	scm_obj_t arg2 = (argc >= 2) ? scm_car(scm_cdr(args)): scm_nil();

	switch (scm_procedure_id(proc)) {
	case SCM_OP_NEWLINE: return scm_newline();
	case SCM_OP_CAR: return scm_car(arg1);
	case SCM_OP_CDR: return scm_cdr(arg1);
	case SCM_OP_IS_PROCEDURE: return scm_boolean(scm_is_procedure(arg1) || scm_is_closure(arg1));
	case SCM_OP_IS_NULL: return scm_boolean(scm_is_null(arg1));
	case SCM_OP_IS_BOOLEAN: return scm_boolean(scm_is_boolean(arg1));
	case SCM_OP_IS_EOF_OBJECT: return scm_boolean(scm_is_eof_object(arg1));
	case SCM_OP_IS_SYMBOL: return scm_boolean(scm_is_symbol(arg1));
	case SCM_OP_IS_STRING: return scm_boolean(scm_is_string(arg1));
	case SCM_OP_IS_PAIR: return scm_boolean(scm_is_pair(arg1));
	case SCM_OP_IS_CHAR: return scm_boolean(scm_is_char(arg1));
	case SCM_OP_IS_NUMBER: return scm_boolean(scm_is_number(arg1));
	case SCM_OP_LENGTH: return scm_number((double)scm_length(arg1));
	case SCM_OP_DISPLAY: return scm_display(arg1);
	case SCM_OP_WRITE: return scm_write(arg1);
	case SCM_OP_LOAD:
		if (!scm_is_string(arg1)) return scm_error("load: takes one string");
		scm_obj_t tmp = scm_load(scm_string_value(arg1));
		return scm_is_error(tmp) ? tmp : scm_unspecified();
	case SCM_OP_IS_ZERO: return scm_is_zero(arg1);
	case SCM_OP_STRING_LENGTH:
		if (!scm_is_string(arg1)) return scm_error("string-length: takes one string");
		return scm_number((double)scm_string_length(arg1));
	case SCM_OP_NUMBER_TO_STRING: return scm_number_to_string(arg1);
	case SCM_OP_IS_EQ: return scm_is_eq(arg1, arg2);
	case SCM_OP_CONS: return scm_cons(arg1, arg2);
	case SCM_OP_SET_CAR: return scm_set_car(arg1, arg2);
	case SCM_OP_SET_CDR: return scm_set_cdr(arg1, arg2);
	case SCM_OP_MODULO: return scm_modulo(arg1, arg2);
	case SCM_OP_QUOTIENT: return scm_quotient(arg1, arg2);
	case SCM_OP_ADD: return scm_add(args);
	case SCM_OP_SUB: return scm_sub(args);
	case SCM_OP_MUL: return scm_mul(args);
	case SCM_OP_DIV: return scm_div(args);
	case SCM_OP_NUMBER_EQ: return scm_number_eq(args);
	case SCM_OP_LT: return scm_lt(args);
	case SCM_OP_GT: return scm_gt(args);
	case SCM_OP_LE: return scm_le(args);
	case SCM_OP_GE: return scm_ge(args);
	case SCM_OP_STRING_EQ: return scm_string_eq(args);
	case SCM_OP_SUBSTRING: return scm_substring(args);
	default: return scm_error("apply: unknown procedure");
	}
}
