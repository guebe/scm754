/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

static void print_list(scm_obj_t obj)
{
	size_t len;

	if ((len = scm_length(obj)) > 100) {
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
		printf("#!%s", scm_procedure_string(obj));
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
