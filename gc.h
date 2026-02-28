/* (c) guenter.ebermann@htl-hl.ac.at */

/* garbage collector helpers */

static inline void scm_gc_init2(uint64_t free_bits[], size_t free_bits_size, size_t *free_idx, uint64_t mark_bits[])
{
	memset(free_bits, 0xFF, free_bits_size);
	memset(mark_bits, 0, free_bits_size);
	*free_idx = 0;
}

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

static inline void scm_gc_sweep(uint64_t free_bits[], size_t free_bits_size, size_t *free_idx, uint64_t mark_bits[])
{
	for (size_t i = 0; i < free_bits_size/sizeof(free_bits[0]); i++) {
		free_bits[i] = ~mark_bits[i];
		mark_bits[i] = 0;
	}
	*free_idx = 0;
}
