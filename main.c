/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"
#include <libgen.h>
#include <unistd.h>

static int load_library(void)
{
	char exe[4096];
	char path[4096];

	ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe));
	if (n < 0 || (size_t)n >= sizeof(exe)) goto errout;

	exe[n] = 0;
	char *dir = dirname(exe);

	int ret = snprintf(path, sizeof(path), "%s/scm754.scm", dir);
	if (ret < 0 || (size_t)ret >= sizeof(path)) goto errout;

	scm_obj_t load_result = scm_load(path);
	if (scm_is_error(load_result)) {
		puts(scm_error_value());
		return -1;
	}
	return 0;
errout:
	puts("cant load library");
	return -1;
}

int main(int argc, char *argv[])
{
	FILE *f;
	bool repl;
	scm_interaction_environment = scm_environment_create();

	if (load_library() < 0) return 1;

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
