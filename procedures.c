#include "scm754.h"
#include <math.h>

extern size_t scm_length(scm_obj_t list)
{
	size_t i = 0;
	while (scm_is_pair(list)) {
		list = scm_cdr(list);
		i++;
	}
	return i;
}

extern scm_obj_t scm_add(scm_obj_t args)
{
	double x = 0.0;
	while (scm_is_pair(args)) {
		scm_obj_t a = scm_car(args);
		if (!scm_is_number(a)) return scm_error("+: needs a number");
		x += scm_number_value(a);
		args = scm_cdr(args);
	}
	return scm_number(x);
}

extern scm_obj_t scm_sub(scm_obj_t args)
{
	scm_obj_t a = scm_car(args);
	if (!scm_is_number(a)) return scm_error("-: needs a number");
	double x = scm_number_value(a);
	args = scm_cdr(args);
	if (scm_is_null(args)) return scm_number(-x);

	while (scm_is_pair(args)) {
		a = scm_car(args);
		if (!scm_is_number(a)) return scm_error("-: needs a number");
		x -= scm_number_value(a);
		args = scm_cdr(args);
	}
	return scm_number(x);
}

extern scm_obj_t scm_mul(scm_obj_t args)
{
	double x = 1.0;
	while (scm_is_pair(args)) {
		scm_obj_t a = scm_car(args);
		if (!scm_is_number(a)) return scm_error("*: needs a number");
		x *= scm_number_value(a);
		args = scm_cdr(args);
	}
	return scm_number(x);
}

extern scm_obj_t scm_div(scm_obj_t args)
{
	scm_obj_t a = scm_car(args);
	if (!scm_is_number(a)) return scm_error("/: needs a number");
	double x = scm_number_value(a);
	args = scm_cdr(args);
	if (scm_is_null(args)) return scm_number(1.0/x);

	while (scm_is_pair(args)) {
		a = scm_car(args);
		if (!scm_is_number(a)) return scm_error("/: needs a number");
		double y = scm_number_value(a);
		if (y == 0.0) return scm_error("/: division by zero");
		x /= y;
		args = scm_cdr(args);
	}
	return scm_number(x);
}

extern scm_obj_t scm_is_zero(scm_obj_t z)
{
	double x = scm_number_value(z);
	return scm_boolean(x == 0.0);
}

extern scm_obj_t scm_quotient(scm_obj_t a, scm_obj_t b)
{
	double x, y;
	if (!scm_is_number(a) || !scm_is_number(b)) return scm_error("quotient: needs two numbers");
	x = scm_number_value(a);
	y = scm_number_value(b);
	if (y == 0.0) return scm_error("quotient: division by zero");
	return scm_number(trunc(x / y));
}

extern scm_obj_t scm_modulo(scm_obj_t a, scm_obj_t b)
{
	double x, y;
	if (!scm_is_number(a) || !scm_is_number(b)) return scm_error("modulo: needs two numbers");
	x = scm_number_value(a);
	y = scm_number_value(b);
	if (y == 0.0) return scm_error("modulo: division by zero");
	return scm_number(x - y * floor(x / y));
}

extern scm_obj_t scm_substring(scm_obj_t args)
{
	scm_obj_t a = scm_car(args);
	if (!scm_is_string(a)) return scm_error("string-copy: needs a string");
	const char *x = scm_string_value(a);

	size_t start = 0;
	size_t max = scm_string_length(a);
	size_t end;
	bool end_avail = false;

	args = scm_cdr(args);
	if (!scm_is_null(args)) {
		scm_obj_t b = scm_car(args);
		if (!scm_is_number(b)) return scm_error("string-copy: needs a number");
		start = (size_t)scm_number_value(b);

		args = scm_cdr(args);
		if (!scm_is_null(args)) {
			scm_obj_t c = scm_car(args);
			if (!scm_is_number(c)) return scm_error("string-copy: needs a number");
			end = (size_t)scm_number_value(c);

			if (!scm_is_null(scm_cdr(args))) return scm_error("string-copy: too many arguments");
			end_avail = true;
		}
	}

	if (!end_avail) end = max;

	if (start > end || end > max) return scm_error("string-copy: invalid arguements");

	return scm_string(x + start, end - start);
}

extern scm_obj_t scm_number_to_string(scm_obj_t number)
{
	char buffer[64];
	int ret = snprintf(buffer, sizeof(buffer), "%.16g", scm_number_value(number));
	if (ret < 0 || (size_t)ret >= sizeof(buffer)) return scm_error("number->string: number too big");
	return scm_string(buffer, (size_t)ret);
}

extern scm_obj_t scm_is_eqv(scm_obj_t a, scm_obj_t b)
{
	if (a == b)
		return scm_true();
	else if (scm_is_number(a) && scm_is_number(b))
		return scm_boolean(scm_number_value(a) == scm_number_value(b));
	else if (scm_is_char(a) && scm_is_char(b))
		return scm_boolean(scm_char_value(a) == scm_char_value(b));
	else
		return scm_false();
}

extern scm_obj_t scm_max(scm_obj_t args)
{
	scm_obj_t a = scm_car(args);
	if (!scm_is_number(a)) return scm_error("max: needs a number");

	double x = scm_number_value(a);
	args = scm_cdr(args);

	while (scm_is_pair(args)) {
		a = scm_car(args);
		if (!scm_is_number(a)) return scm_error("max: needs a number");
		double y = scm_number_value(a);
		if (y > x) x = y;
		args = scm_cdr(args);
	}
	return scm_number(x);
}

#define SCM_COMPARE(name, sname, type, is_t, get_v, cmp)                  \
scm_obj_t name(scm_obj_t args) {                                          \
    scm_obj_t o = scm_car(args);                                          \
    if (!is_t(o)) return scm_error(sname ": type err");                   \
    type x = get_v(o);                                                    \
    for (args = scm_cdr(args); scm_is_pair(args); args = scm_cdr(args)) { \
        o = scm_car(args);                                                \
        if (!is_t(o)) return scm_error(sname ": type err");               \
        type y = get_v(o);                                                \
        if (!(cmp(x, y))) return scm_false();                             \
        x = y;                                                            \
    }                                                                     \
    return scm_true();                                                    \
}

#define SCM_CHAR_CI_VALUE(x) tolower(scm_char_value(x))
#define SCM_CMP_LT(x, y) (x < y)
#define SCM_CMP_GT(x, y) (x > y)
#define SCM_CMP_LE(x, y) (x <= y)
#define SCM_CMP_GE(x, y) (x >= y)
#define SCM_CMP_EQ(x, y) (x == y)
#define SCM_CMP_STRING(x, y) (strcmp(x, y) == 0)
SCM_COMPARE(scm_char_lt, "char<?", int, scm_is_char, scm_char_value, SCM_CMP_LT)
SCM_COMPARE(scm_char_gt, "char>?", int, scm_is_char, scm_char_value, SCM_CMP_GT)
SCM_COMPARE(scm_char_le, "char<=?", int, scm_is_char, scm_char_value, SCM_CMP_LE)
SCM_COMPARE(scm_char_ge, "char>=?", int, scm_is_char, scm_char_value, SCM_CMP_GE)
SCM_COMPARE(scm_char_eq, "char=?", int, scm_is_char, scm_char_value, SCM_CMP_EQ)
SCM_COMPARE(scm_char_ci_lt, "char-ci<?", int, scm_is_char, SCM_CHAR_CI_VALUE, SCM_CMP_LT)
SCM_COMPARE(scm_char_ci_gt, "char-ci>?", int, scm_is_char, SCM_CHAR_CI_VALUE, SCM_CMP_GT)
SCM_COMPARE(scm_char_ci_le, "char-ci<=?", int, scm_is_char, SCM_CHAR_CI_VALUE, SCM_CMP_LE)
SCM_COMPARE(scm_char_ci_ge, "char-ci>=?", int, scm_is_char, SCM_CHAR_CI_VALUE, SCM_CMP_GE)
SCM_COMPARE(scm_char_ci_eq, "char-ci=?", int, scm_is_char, SCM_CHAR_CI_VALUE, SCM_CMP_EQ)
SCM_COMPARE(scm_number_lt, "<", double, scm_is_number, scm_number_value, SCM_CMP_LT)
SCM_COMPARE(scm_number_gt, ">", double, scm_is_number, scm_number_value, SCM_CMP_GT)
SCM_COMPARE(scm_number_le, "<=", double, scm_is_number, scm_number_value, SCM_CMP_LE)
SCM_COMPARE(scm_number_ge, ">=", double, scm_is_number, scm_number_value, SCM_CMP_GE)
SCM_COMPARE(scm_number_eq, "=", double, scm_is_number, scm_number_value, SCM_CMP_EQ)
SCM_COMPARE(scm_string_eq, "string=?", const char *, scm_is_string, scm_string_value, SCM_CMP_STRING)
