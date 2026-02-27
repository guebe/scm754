/* (c) guenter.ebermann@htl-hl.ac.at */

/* garbage collector helpers */

static inline size_t scm_gc_alloc(uint64_t free_bits[], size_t free_bits_size, size_t *idx) {
	for (size_t i = *idx; i < free_bits_size/sizeof(free_bits[0]); i++) {
		if (free_bits[i]) {
			int bit = __builtin_ctzll(free_bits[i]); /* count trailing zeros */
			free_bits[i] &= free_bits[i] - 1;
			*idx = i;
			return i * 64 + (size_t)bit;
		}
	}
	return free_bits_size;
}
