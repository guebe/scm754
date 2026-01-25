/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

int LLVMFuzzerInitialize(void)
{
	//scm_interaction_environment = scm_environment_create();
	scm_gc_init();
	return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	FILE *mem = fmemopen((void *)data, size, "r");
	if (!mem) return 0;
	scm_current_input_port = mem;
	scm_read();
	scm_gc_collect();
	fclose(mem);
	return 0;
}
