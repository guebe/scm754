/* (c) guenter.ebermann@htl-hl.ac.at */
#ifndef __GC_H__
#define __GC_H__

/* garbage collector implementation */

typedef struct
{
	scm_obj_t car;
	scm_obj_t cdr;
} scm_pair_t;

#define SCM_CELL_NUM 32768U
_Static_assert(SCM_CELL_NUM % 64 == 0, "SCM_CELL_NUM must be multiple of 64");
extern scm_pair_t cell[SCM_CELL_NUM];
extern uint64_t cell_free_bits[SCM_CELL_NUM/64];
extern size_t cell_free_index;

#define SCM_STRING_NUM 2048U
_Static_assert(SCM_STRING_NUM % 64 == 0, "SCM_STRING_NUM must be multiple of 64");
extern char *strings[SCM_STRING_NUM];
extern uint64_t string_free_bits[SCM_STRING_NUM/64];
extern size_t string_free_index;

static inline size_t scm_gc_alloc(uint64_t free_bits[], size_t free_bits_size, size_t *free_idx)
{
	for (size_t i = *free_idx; i < free_bits_size/sizeof(free_bits[0]); i++) {
		if (free_bits[i]) {
			int bit = __builtin_ctzll(free_bits[i]); /* count trailing zeros */
			free_bits[i] &= free_bits[i] - 1;
			*free_idx = i;
			return i * 64 + (size_t)bit;
		}
	}
	return free_bits_size;
}

extern void scm_gc_init(void);
extern void scm_gc_deinit(void);
extern void scm_gc_collect(void);
extern void scm_gc_push(const scm_obj_t *obj);
extern void scm_gc_pop(void);
extern void scm_gc_push2(const scm_obj_t *obj1, const scm_obj_t *obj2);
extern void scm_gc_pop2(void);
#endif
