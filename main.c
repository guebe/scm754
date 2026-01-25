/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"

int main(int argc, char *argv[])
{
	FILE *f;
	bool repl;
	scm_interaction_environment = scm_environment_create();

	if (argc != 2) {
		repl = true;
		scm_current_input_port = stdin;
	}
	else {
		repl = false;
		f = fopen(argv[1], "r");
		if (f == NULL) {
			printf("cant open file %s\n", argv[1]);
			return 1;
		}
		scm_current_input_port = f;
	}

	while (1) {
		if (repl) {
			fputs("> ", stdout);
			fflush(stdout);
		}
		scm_gc_collect();
		scm_obj_t obj = scm_read();
		if (scm_is_eof_object(obj)) { break; }
		else if (scm_is_error(obj)) { puts(scm_error_value()); if (repl) continue; else break; }

#ifndef SCM_NO_EVAL
		obj = scm_eval(obj, scm_interaction_environment);
		if (scm_is_error(obj)) { puts(scm_error_value()); if (repl) continue; else break; }
#endif

		/* unspecified is returned by e.g. define as per specification. we do _not_ want to print the unspecified value in a REPL */
		if (!scm_is_unspecified(obj)) {
			scm_write(obj);
			putchar('\n');
		}
	}

	if (!repl) fclose(f);
	return 0;
}
