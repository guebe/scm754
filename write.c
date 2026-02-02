/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

static const char *procedure_string(scm_obj_t proc)
{
	uint32_t id = scm_procedure_id(proc);

	switch(id) {
	case SCM_OP_NEWLINE: return "newline";
	case SCM_OP_CAR: return "car";
	case SCM_OP_CDR: return "cdr";
	case SCM_OP_IS_PROCEDURE: return "procedure?";
	case SCM_OP_IS_NULL: return "null?";
	case SCM_OP_IS_BOOLEAN: return "boolean?";
	case SCM_OP_IS_EOF_OBJECT: return "eof-object?";
	case SCM_OP_IS_SYMBOL: return "symbol?";
	case SCM_OP_IS_STRING: return "string?";
	case SCM_OP_IS_PAIR: return "pair?";
	case SCM_OP_IS_CHAR: return "char?";
	case SCM_OP_IS_NUMBER: return "number?";
	case SCM_OP_LENGTH: return "length";
	case SCM_OP_DISPLAY: return "display";
	case SCM_OP_LOAD: return "load";
	case SCM_OP_IS_ZERO: return "zero?";
	case SCM_OP_STRING_LENGTH: return "string-length";
	case SCM_OP_NUMBER_TO_STRING: return "number->string";
	case SCM_OP_IS_EQ: return "eq?";
	case SCM_OP_CONS: return "cons";
	case SCM_OP_SET_CAR: return "set-car!";
	case SCM_OP_SET_CDR: return "set-cdr!";
	case SCM_OP_MODULO: return "modulo";
	case SCM_OP_QUOTIENT: return "quotient";
	case SCM_OP_ADD: return "+";
	case SCM_OP_SUB: return "-";
	case SCM_OP_MUL: return "*";
	case SCM_OP_DIV: return "/";
	case SCM_OP_WRITE: return "write";
	case SCM_OP_NUMBER_EQ: return "=";
	case SCM_OP_LT: return "<=";
	case SCM_OP_GT: return ">=";
	case SCM_OP_LE: return "<";
	case SCM_OP_GE: return ">";
	case SCM_OP_STRING_EQ: return "string=?";
	case SCM_OP_SUBSTRING: return "substring";
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
