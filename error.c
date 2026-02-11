/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

extern scm_obj_t scm_error(const char *message, ...)
{
	va_list ap;

	fputs("; error: ", stdout);
	va_start(ap, message);
	vprintf(message, ap);
	va_end(ap);
	putchar('\n');

	return SCM_ERROR;
}

extern void scm_fatal(const char *message)
{
	puts(message);
	abort();
}
