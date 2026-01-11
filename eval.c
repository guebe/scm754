/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scheme.h"

extern scm_obj_t scm_eval(scm_obj_t expr_or_def, scm_obj_t environment_specifier)
{
	scm_obj_t op, args, string;
	const char *symbol;
	size_t symbol_length;

	if (scm_is_empty_list(expr_or_def)) {
		return scm_error("eval: can not apply empty-list-object ()");
	}
	else if (scm_is_symbol(expr_or_def)) {
		/* variable */
		return scm_error("lookup(environment_specifier, expr_or_def) not yet supported");
	}
	else if (scm_is_pair(expr_or_def)) {
		op = scm_car(expr_or_def);
		args = scm_cdr(expr_or_def);

		if (scm_is_symbol(op)) {
			/* special forms */
			string = scm_symbol_to_string(op);
			symbol = scm_string_value(string);
			symbol_length = scm_string_length(string);

			if ((symbol_length == 2) && (memcmp(symbol, "if", 2) == 0)) {
				return scm_error("eval: if not implemented");
			}
			else if ((symbol_length == 5) && (memcmp(symbol, "quote", 5) == 0)) {
				return scm_is_empty_list(scm_cdr(args)) ? scm_car(args) : scm_error("eval: bad quote form");
			}
			else if ((symbol_length == 6) && (memcmp(symbol, "define", 6) == 0)) {
				return scm_error("eval: define not implemented");
			}
			else if ((symbol_length == 6) && (memcmp(symbol, "lambda", 6) == 0)) {
				return scm_error("eval: lambda not implemented");
			}
		}

		scm_obj_t proc = scm_eval(op, environment_specifier);
		if (scm_is_error_object(proc)) return proc;
		return scm_apply(proc, args, environment_specifier);
		/* FIXME: all checked in apply? else return scm_error("eval: can not apply non-procedure"); */
	}
	else {
		/* self-evaluating */
	       	return expr_or_def;
	}
}

extern scm_obj_t scm_apply(scm_obj_t proc, scm_obj_t args, scm_obj_t environment_specifier)
{
	(void)proc;
	(void)args;
	(void)environment_specifier;

	return scm_error("apply: not implemented");
}
