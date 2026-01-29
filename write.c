/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

static const char *procedure_string(scm_obj_t proc)
{
	scm_procedure_t id = scm_procedure_id(proc);

	switch(id) {
	case SCM_PROCEDURE_NEWLINE: return "newline";
	case SCM_PROCEDURE_CAR: return "car";
	case SCM_PROCEDURE_CDR: return "cdr";
	case SCM_PROCEDURE_IS_PROCEDURE: return "procedure?";
	case SCM_PROCEDURE_IS_NULL: return "null?";
	case SCM_PROCEDURE_IS_BOOLEAN: return "boolean?";
	case SCM_PROCEDURE_IS_EOF_OBJECT: return "eof-object?";
	case SCM_PROCEDURE_IS_SYMBOL: return "symbol?";
	case SCM_PROCEDURE_IS_STRING: return "string?";
	case SCM_PROCEDURE_IS_PAIR: return "pair?";
	case SCM_PROCEDURE_IS_CHAR: return "char?";
	case SCM_PROCEDURE_IS_NUMBER: return "number?";
	case SCM_PROCEDURE_LENGTH: return "length";
	case SCM_PROCEDURE_DISPLAY: return "display";
	case SCM_PROCEDURE_LOAD: return "load";
	case SCM_PROCEDURE_IS_ZERO: return "zero?";
	case SCM_PROCEDURE_STRING_LENGTH: return "string-length";
	case SCM_PROCEDURE_NUMBER_TO_STRING: return "number->string";
	case SCM_PROCEDURE_IS_EQ: return "eq?";
	case SCM_PROCEDURE_CONS: return "cons";
	case SCM_PROCEDURE_SET_CAR: return "set-car!";
	case SCM_PROCEDURE_SET_CDR: return "set-cdr!";
	case SCM_PROCEDURE_MODULO: return "modulo";
	case SCM_PROCEDURE_QUOTIENT: return "quotient";
	case SCM_PROCEDURE_ADD: return "+";
	case SCM_PROCEDURE_SUB: return "-";
	case SCM_PROCEDURE_MUL: return "*";
	case SCM_PROCEDURE_DIV: return "/";
	case SCM_PROCEDURE_WRITE: return "write";
	case SCM_PROCEDURE_NUMBER_EQ: return "=";
	case SCM_PROCEDURE_LT: return "<=";
	case SCM_PROCEDURE_GT: return ">=";
	case SCM_PROCEDURE_LE: return "<";
	case SCM_PROCEDURE_GE: return ">";
	case SCM_PROCEDURE_STRING_EQ: return "string=?";
	case SCM_PROCEDURE_SUBSTRING: return "substring";
	default: return "<unknown>";
	}
}

static void print_list(scm_obj_t obj)
{
	size_t len;

	if ((len = scm_length(obj)) >= 20) {
		printf("(<toolong %lu>)", len);
		return;
	}
	putchar('(');
	while (1) {
		scm_write(scm_car(obj));
		obj = scm_cdr(obj);
		if (!scm_is_pair(obj)) break;
		putchar(' ');
	}
	if (!scm_is_null(obj)) {
		fputs(" . ", stdout);
		scm_write(obj);
	}
	putchar(')');
}

extern void print(scm_obj_t obj, bool readable)
{
	if (scm_is_null(obj)) {
		fputs("()", stdout);
	}
	else if (scm_is_boolean(obj)) {
		fputs(scm_boolean_value(obj) ? "#t" : "#f", stdout);
	}
	else if (scm_is_eof_object(obj)) {
		fputs("#!eof", stdout);
	}
	else if (scm_is_dot(obj)) {
		fputs("#!dot", stdout);
	}
	else if (scm_is_rparen(obj)) {
		fputs("#!rparen", stdout);
	}
	else if (scm_is_unspecified(obj)) {
		fputs("#!unspecified", stdout);
	}
	else if (scm_is_error(obj)) {
		fputs("#!error", stdout);
	}
	else if (scm_is_procedure(obj)) {
		printf("#!%s", procedure_string(obj));
	}
	else if (scm_is_closure(obj)) {
		fputs("#!closure", stdout);
	}
	else if (scm_is_symbol(obj)) {
		fputs(scm_string_value(scm_symbol_to_string(obj)), stdout);
	}
	else if (scm_is_string(obj)) {
		if (readable) putchar('\"');
		fputs(scm_string_value(obj), stdout);
		if (readable) putchar('\"');
	}
	else if (scm_is_pair(obj)) {
		print_list(obj);
	}
	else if (scm_is_char(obj)) {
		fputs("#\\", stdout);
		putchar(scm_char_value(obj));
	}
	else {
		printf("%.16g", scm_number_value(obj));
	}
}

extern scm_obj_t scm_write(scm_obj_t obj)
{
	print(obj, true);
	return scm_unspecified();
}

extern scm_obj_t scm_display(scm_obj_t obj)
{
	print(obj, false);
	return scm_unspecified();
}

extern scm_obj_t scm_newline(void)
{
	putchar('\n');
	return scm_unspecified();
}
