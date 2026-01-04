/* (c) guenter.ebermann@htl-hl.ac.at
 * Tiny Scheme Interpreter
 */
#ifndef __CELL_H__
#define __CELL_H__

#include <string.h>

#define SCM_CELL_NUM 8192U /* number of cells */

extern double scm_cell[SCM_CELL_NUM];
extern size_t scm_i;

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
