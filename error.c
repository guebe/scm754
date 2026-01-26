/* (c) guenter.ebermann@htl-hl.ac.at */
#include "scm754.h"

static char last_error_message[256] = "; error: unknown";

extern scm_obj_t scm_error(const char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vsnprintf(last_error_message + 9, sizeof last_error_message - 9, message, ap);
	va_end(ap);

	return SCM_ERROR;
}

extern const char *scm_error_value(void)
{
	return last_error_message;
}
