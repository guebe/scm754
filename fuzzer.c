/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

extern scm_obj_t scm_error(const char *message, ...)
{
	(void)message;
	return SCM_ERROR;
}

extern void scm_fatal(const char *message)
{
	puts(message);
	abort();
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	FILE *mem = fmemopen((void *)data, size, "r");
	if (!mem) return 0;
	scm_interaction_environment = scm_env_create();
	scm_current_input_port = mem;
	scm_read();
	scm_gc_collect();
	scm_gc_string_free();
	fclose(mem);
	return 0;
}
