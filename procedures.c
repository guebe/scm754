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

extern scm_obj_t scm_add(scm_obj_t argv[], size_t argc)
{
	double x = 0.0;
	for (size_t i = 0; i < argc; i++) {
		scm_obj_t a = argv[i];
		if (!scm_is_number(a)) return scm_error("+: needs a number");
		x += scm_number_value(a);
	}
	return scm_number(x);
}

extern scm_obj_t scm_sub(scm_obj_t argv[], size_t argc)
{
	if (argc == 0) return scm_error("-: needs an argument");
	scm_obj_t a = argv[0];
	if (!scm_is_number(a)) return scm_error("-: needs a number");
	double x = scm_number_value(a);
	if (argc == 1) return scm_number(-x);

	for (size_t i = 1; i < argc; i++) {
		a = argv[i];
		if (!scm_is_number(a)) return scm_error("-: needs a number");
		x -= scm_number_value(a);
	}
	return scm_number(x);
}

extern scm_obj_t scm_mul(scm_obj_t argv[], size_t argc)
{
	double x = 1.0;
	for (size_t i = 0; i < argc; i++) {
		scm_obj_t a = argv[i];
		if (!scm_is_number(a)) return scm_error("*: needs a number");
		x *= scm_number_value(a);
	}
	return scm_number(x);
}

extern scm_obj_t scm_div(scm_obj_t argv[], size_t argc)
{
	if (argc == 0) return scm_error("/: needs an argument");
	scm_obj_t a = argv[0];
	if (!scm_is_number(a)) return scm_error("/: needs a number");
	double x = scm_number_value(a);
	if (argc == 1) return scm_number(1.0/x);

	for (size_t i = 1; i < argc; i++) {
		a = argv[i];
		if (!scm_is_number(a)) return scm_error("/: needs a number");
		double y = scm_number_value(a);
		if (y == 0.0) return scm_error("/: division by zero");
		x /= y;
	}
	return scm_number(x);
}

extern scm_obj_t scm_numeric_equal(scm_obj_t argv[], size_t argc)
{
	if (argc == 0) return scm_error("=: needs an argument");
	scm_obj_t a = argv[0];
	if (!scm_is_number(a)) return scm_error("=: needs a number");
	double x = scm_number_value(a);
	if (argc == 1) return scm_true();

	for (size_t i = 1; i < argc; i++) {
		a = argv[i];
		if (!scm_is_number(a)) return scm_error("=: needs a number");
		if (x != scm_number_value(a)) return scm_false();
	}
	return scm_true();
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
