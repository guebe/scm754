#include "scm754.h"

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
	size_t i = scm_number_to_size(k);
	if (i == SIZE_MAX) return scm_error("list-ref: can't convert to size_t");
	for (size_t j = 0; j < i; j++) {
		if (!scm_is_pair(list)) goto err;
		list = scm_cdr(list);
	}
	if (!scm_is_pair(list)) goto err;
	return scm_car(list);
err:
	return scm_error("list-ref: index %lu out of bounds", i);
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

	if (scm_is_vector(obj1) && scm_is_vector(obj2)) {
		size_t k1 = scm_vector_length(obj1);
		size_t k2 = scm_vector_length(obj2);
		if (k1 != k2) return false;
		for (size_t i = 0; i < k1; i++) {
			if (!scm_is_equal(scm_vector_ref(obj1, i),
					  scm_vector_ref(obj2, i)))
				return false;
		}
		return true;
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

#define SCM_ASSOC(name, cmp)                                      \
scm_obj_t name(scm_obj_t obj, scm_obj_t alist)                    \
{                                                                 \
	while (scm_is_pair(alist)) {                              \
		scm_obj_t pair = scm_car(alist);                  \
		if (scm_is_pair(pair) && cmp(obj, scm_car(pair))) \
		return pair;                                      \
		alist = scm_cdr(alist);                           \
	}                                                         \
	return scm_false();                                       \
}

SCM_ASSOC(scm_assq,  scm_is_eq)
SCM_ASSOC(scm_assv,  scm_is_eqv)
SCM_ASSOC(scm_assoc, scm_is_equal)
