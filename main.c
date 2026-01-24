/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"
#include <stdio.h>

int main(void)
{
	scm_obj_t obj;

	scm_interaction_environment = scm_environment_create();

	while (1) {
		fputs("> ", stdout);
		fflush(stdout);

		scm_gc_collect();

		obj = scm_read();
		if (scm_is_eof_object(obj)) { break; }
		else if (scm_is_error(obj)) { puts(scm_error_value()); continue; }

#ifndef SCM_NO_EVAL
		obj = scm_eval(obj, scm_interaction_environment);
		if (scm_is_error(obj)) { puts(scm_error_value()); continue; }
#endif

		/* unspecified is returned by e.g. define as per specification. we do _not_ want to print the unspecified value in a REPL */
		if (!scm_is_unspecified(obj)) {
			scm_write(obj);
			putchar('\n');
		}
	}

	putchar('\n');
	return 0;
}
