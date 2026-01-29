/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

static bool diagnostic = false;

extern void scm_enable_error(void)
{
	diagnostic = true;
}

extern scm_obj_t scm_error(const char *message, ...)
{
	va_list ap;

	if (diagnostic)
	{
		va_start(ap, message);
		fputs("; error: ", stdout);
		vprintf(message, ap);
		putchar('\n');
		va_end(ap);
	}

	return SCM_ERROR;
}
