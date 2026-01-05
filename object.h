/* (c) guenter.ebermann@htl-hl.ac.at
 * Tiny Scheme Interpreter
 *
 * Scheme implementations object representation
 */

#ifndef __OBJECT_H__
#define __OBJECT_H__

/* NaN boxing */

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

/* Memory handling */

#define SCM_CELL_NUM 8192U
static double scm_cell[SCM_CELL_NUM];
static size_t scm_i = 0;

static inline size_t scm_cell_put(const char *buf, size_t size) {
	size_t i = scm_i;
	size_t free = SCM_CELL_NUM - i;

	if (size * 8 <= free) {
		memcpy(&scm_cell[i], buf, size);
		scm_i = i + (size + 7) / 8;
		return i;
	}

	return SCM_CELL_NUM;
}

#endif
