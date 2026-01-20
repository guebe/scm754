/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"
#include <stdio.h>

static void write_list(scm_obj_t obj)
{
	putchar('(');
	while (1) {
		scm_write(scm_car(obj));
		obj = scm_cdr(obj);
		if (!scm_is_pair(obj)) break;
		putchar(' ');
	}
	if (!scm_is_empty_list(obj)) {
		fputs(" . ", stdout);
		scm_write(obj);
	}
	putchar(')');
}

static void write_string(scm_obj_t string)
{
	fwrite(scm_string_value(string), 1, scm_string_length(string), stdout);
}

extern scm_obj_t scm_write(scm_obj_t obj)
{
	if (scm_is_empty_list(obj)) {
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
	else if (scm_is_error_object(obj)) {
		fputs("#!error", stdout);
	}
	else if (scm_is_procedure(obj)) {
		printf("#!procedure%u", scm_procedure_id(obj));
	}
	else if (scm_is_closure(obj)) {
		fputs("#!closure", stdout);
		write_list((obj & ~SCM_MASK) | SCM_PAIR);
	}
	else if (scm_is_symbol(obj)) {
		write_string(scm_symbol_to_string(obj));
	}
	else if (scm_is_string(obj)) {
		putchar('\"');
	       	write_string(obj);
	       	putchar('\"');
	}
	else if (scm_is_pair(obj)) {
	       	write_list(obj);
	}
       	else if (scm_is_char(obj)) {
		fputs("#\\", stdout);
		putchar(scm_char_value(obj));
	}
	else {
		printf("%.16g", scm_number_value(obj));
	}
	return scm_unspecified();
}
