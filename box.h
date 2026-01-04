/* (c) guenter.ebermann@htl-hl.ac.at
 * Tiny Scheme Interpreter
 *
 * NaN boxing
 */
#ifndef __BOX_H__
#define __BOX_H__

#include <inttypes.h>
#include <string.h>

#define SCM_NIL 0xfff8U /* '() - the empty list */
#define SCM_BOOLEAN 0xfff9U /* LSB: 0 ... #f, 1 ... #t */
#define SCM_SYMBOL 0xfffaU /* index to len/8 cells */
#define SCM_STRING 0xfffbU /* index to len/8 cells */
#define SCM_CHARACTER 0xfffcU /* LSB: character */
#define SCM_LIST 0xfffdU /* list: index to two cells */

#define SCM_TAG_SHIFT 48U
#define SCM_TAG_MASK 0xffffU

#define SCM_VALUE_MASK 0xffffffffffffULL

static inline uint64_t scm_value(uint64_t x)
{
	return x & SCM_VALUE_MASK;
}

static inline unsigned int scm_get_tag(uint64_t x)
{
	return (x >> SCM_TAG_SHIFT) & SCM_TAG_MASK;
}

static inline uint64_t scm_set_tag(unsigned int x)
{
	return ((uint64_t)(x & SCM_TAG_MASK)) << SCM_TAG_SHIFT;
}

static inline double scm_box(unsigned int tag, uint64_t value)
{
	double d;
	value = scm_set_tag(tag) | scm_value(value);
	memcpy(&d, &value, sizeof(d));
	return d;
}

static inline uint64_t scm_unbox(double d)
{
	uint64_t u;
	memcpy(&u, &d, sizeof(u));
	return u;
}

#endif
