/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"
#include <errno.h>
#include <stdlib.h>

/* R7RS, section 6.2.7, Numerical input and output */
extern scm_obj_t scm_string_to_number(const char *string, int radix)
{
	char *end;
	double value;

	errno = 0;

	if (radix > 0)
		value = (double)strtol(string, &end, radix);
	else
		value = strtod(string, &end);

	if ((end == string) || (*end != '\0') || (errno != 0)) return scm_false();

	return scm_number(value);
}

extern scm_obj_t scm_number_to_string(scm_obj_t number)
{
	if (!scm_is_number(number)) return scm_error("number->string: needs a number");
	char buffer[64];
	int ret = snprintf(buffer, sizeof(buffer), "%.16g", scm_number_value(number));
	if (ret < 0 || (size_t)ret >= sizeof(buffer)) return scm_error("number->string: number too big");
	return scm_string(buffer, (size_t)ret);
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

extern scm_obj_t scm_is_zero(scm_obj_t z)
{
	if (!scm_is_number(z)) return scm_error("zero?: needs a number");
	double x = scm_number_value(z);
	return scm_boolean(x == 0.0);
}

