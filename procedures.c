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
	if (!scm_is_pair(args)) return scm_error("-: needs an argument");
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
	if (!scm_is_pair(args)) return scm_error("/: needs an argument");
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

typedef bool (*cmp_func_t)(double, double);
static bool lt(double a, double b) { return a < b; }
static bool gt(double a, double b) { return a > b; }
static bool le(double a, double b) { return a <= b; }
static bool ge(double a, double b) { return a >= b; }
static bool eq(double a, double b) { return a == b; }

static scm_obj_t number_compare(scm_obj_t args, cmp_func_t cmp, const char *name)
{
	if (!scm_is_pair(args)) return scm_error("%s: needs an argument", name);

	scm_obj_t a = scm_car(args);
	if (!scm_is_number(a)) return scm_error("%s: needs a number", name);
	double x = scm_number_value(a);

	args = scm_cdr(args);
	while (scm_is_pair(args)) {
		a = scm_car(args);
		if (!scm_is_number(a)) return scm_error("%s: needs a number", name);
		double y = scm_number_value(a);
		if (!cmp(x, y)) return scm_false();
		x = y;
		args = scm_cdr(args);
	}

	return scm_true();
}

extern scm_obj_t scm_lt(scm_obj_t args) { return number_compare(args, lt, "<"); }
extern scm_obj_t scm_gt(scm_obj_t args) { return number_compare(args, gt, ">"); }
extern scm_obj_t scm_le(scm_obj_t args) { return number_compare(args, le, "<="); }
extern scm_obj_t scm_ge(scm_obj_t args) { return number_compare(args, ge, ">="); }
extern scm_obj_t scm_number_eq(scm_obj_t args) { return number_compare(args, eq, "="); }

extern scm_obj_t scm_string_eq(scm_obj_t args)
{
	if (!scm_is_pair(args)) return scm_error("string=?: needs an argument");

	scm_obj_t a = scm_car(args);
	if (!scm_is_string(a)) return scm_error("string=?: needs a string");
	const char *x = scm_string_value(a);

	args = scm_cdr(args);
	while (scm_is_pair(args)) {
		a = scm_car(args);
		if (!scm_is_string(a)) return scm_error("string=?: needs a string");
		const char *y = scm_string_value(a);
		if (strcmp(x, y) != 0) return scm_false();
		x = y;
		args = scm_cdr(args);
	}

	return scm_true();
}

extern scm_obj_t scm_string_copy(scm_obj_t args)
{
	if (!scm_is_pair(args)) return scm_error("string-copy: needs an argument");

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
