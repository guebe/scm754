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

extern scm_obj_t scm_list_ref(scm_obj_t list, scm_obj_t k)
{
	if (!scm_is_number(k)) return scm_error("list-ref: type err");
	size_t i = (size_t)scm_number_value(k);
	for (size_t j = 0; j < i; j++) {
		if (!scm_is_pair(list)) goto err;
		list = scm_cdr(list);
	}
	if (!scm_is_pair(list)) goto err;
	return scm_car(list);
err:
	return scm_error("list-ref: index %lu out of bounds", i);
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

extern scm_obj_t scm_string_ref(scm_obj_t string, scm_obj_t k)
{
	if (!scm_is_string(string) || !scm_is_number(k)) return scm_error("string-ref: type err");

	const char *s = scm_string_value(string);
	size_t len = scm_string_length(string);
	size_t i = (size_t)scm_number_value(k);

	if (i >= len)
		return scm_error("string-ref: index %lu out of bounds (string length %lu)", i, len);

	return scm_char(s[i]);
}

extern scm_obj_t scm_string_set(scm_obj_t string, scm_obj_t k, scm_obj_t c)
{
	if (!scm_is_string(string) || !scm_is_number(k) || !scm_is_char(c)) return scm_error("string-set!: type err");

	char *s = scm_string_value(string);
	size_t len = scm_string_length(string);
	size_t i = (size_t)scm_number_value(k);

	if (i >= len)
		return scm_error("string-set!: index %lu out of bounds (string length %lu)", i, len);

	s[i] = (char)scm_char_value(c);
	return scm_unspecified();
}

extern scm_obj_t scm_is_zero(scm_obj_t z)
{
	if (!scm_is_number(z)) return scm_error("zero?: needs a number");
	double x = scm_number_value(z);
	return scm_boolean(x == 0.0);
}

extern scm_obj_t scm_number_to_string(scm_obj_t number)
{
	if (!scm_is_number(number)) return scm_error("number->string: needs a number");
	char buffer[64];
	int ret = snprintf(buffer, sizeof(buffer), "%.16g", scm_number_value(number));
	if (ret < 0 || (size_t)ret >= sizeof(buffer)) return scm_error("number->string: number too big");
	return scm_string(buffer, (size_t)ret);
}

#define SCM_COMPARE(name, sname, type, is_t, get_v, cmp)                  \
scm_obj_t name(scm_obj_t args)                                            \
{                                                                         \
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

extern bool scm_is_equal(scm_obj_t obj1, scm_obj_t obj2)
{
tail_recurse:
	if (scm_is_eqv(obj1, obj2)) return true;

	if (scm_is_pair(obj1) && scm_is_pair(obj2)) {
		if (!scm_is_equal(scm_car(obj1), scm_car(obj2))) return false;
		obj1 = scm_cdr(obj1);
		obj2 = scm_cdr(obj2);
		goto tail_recurse;
	}

	if (scm_is_string(obj1) && scm_is_string(obj2))
		return (strcmp(scm_string_value(obj1), scm_string_value(obj2)) == 0);

	return false;
}

#define SCM_MEMBER(name, cmp)                    \
scm_obj_t name(scm_obj_t obj, scm_obj_t list)    \
{                                                \
	while (scm_is_pair(list)) {              \
		scm_obj_t item = scm_car(list);  \
		if (cmp(obj, item)) return list; \
		list = scm_cdr(list);            \
	}                                        \
	return scm_false();                      \
}

SCM_MEMBER(scm_memq, scm_is_eq)
SCM_MEMBER(scm_memv, scm_is_eqv)
SCM_MEMBER(scm_member, scm_is_equal)
