/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"
#include <stdio.h>

/* Environment which is a list of frames,
 * whereas each frame is a list of pairs (symbol-index . <value|procedure>).
 * This is needed to support nested environments and rebinding of variables.
 * Example: (((a . 4) (b . proc)) ((a . 5) (b . 8)))
 *
 * Note: all symbols stored in the environment are globally unique (interned in the global_symbols list).
 * This is to avoid searching by string-compare in the environment. By using
 * interned symbols we can search in the environment using numeric equality
 * instead of string comparison. Even though symbols are globally unique, each
 * symbol can have a different binding in every environment frame! */
static scm_obj_t interaction_environment;

int main(void)
{
	scm_obj_t obj;

	interaction_environment = scm_environment_create();

	while (1) {
		fputs("> ", stdout);
		fflush(stdout);

		obj = scm_read();
		if (scm_is_eof_object(obj)) { break; }
		else if (scm_is_error(obj)) { puts(scm_error_value()); continue; }

#ifndef SCM_NO_EVAL
		obj = scm_eval(obj, interaction_environment);
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
