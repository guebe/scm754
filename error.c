/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scheme.h"
#include <stdio.h>

static char last_error_message[256] = "; error: ";

extern scm_obj_t scm_error(const char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	vsnprintf(last_error_message + 9, sizeof last_error_message - 9, message, ap);
	va_end(ap);

	return SCM_ERROR;
}

extern const char *scm_error_object_message(scm_obj_t error_object)
{
	(void) error_object;
	return last_error_message;
}
