/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

extern size_t scm_vector_length(scm_obj_t vec)
{
	if (!scm_is_vector(vec)) {
		(void)scm_error("error: vector-length: not a vector");
		return 0;
	}
	size_t i = (uint32_t)vec;
	assert(i < SCM_VECTOR_NUM);
	return vector[i].len;
}

extern scm_obj_t scm_vector_ref(scm_obj_t vec, size_t k)
{
	if (!scm_is_vector(vec)) return scm_error("vector-ref: not a vector");
	size_t i = (uint32_t)vec;
	assert(i < SCM_VECTOR_NUM);
	if (k >= vector[i].len) return scm_error("vector-ref: index out of range");
	return vector[i].obj[k];
}

extern scm_obj_t scm_vector_set(scm_obj_t vec, size_t k, scm_obj_t value)
{
	if (!scm_is_vector(vec)) return scm_error("vector-set!: not a vector");
	size_t i = (uint32_t)vec;
	assert(i < SCM_VECTOR_NUM);
	if (k >= vector[i].len) return scm_error("vector-set!: index out of range");
	vector[i].obj[k] = value;
	return scm_unspecified();
}

extern scm_obj_t scm_make_vector(size_t k, scm_obj_t fill)
{
	size_t i = scm_gc_alloc(vector_free_bits, sizeof(vector_free_bits), &vector_free_index);
	if (i >= SCM_VECTOR_NUM) scm_fatal("out of vector memory");

	scm_obj_t *obj = vector[i].obj;

	obj = realloc(obj, k*sizeof(scm_obj_t));
	if (obj == NULL) scm_fatal("vector allocation failed");
	for (size_t j = 0; j < k; j++)
		obj[j] = fill;

	vector[i].obj = obj;
	vector[i].len = k;
	return SCM_VECTOR | i;
}
