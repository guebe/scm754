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

	scm_interaction_environment = scm_env_create();

	if (load_library() < 0) return 1;

	if (argc < 2) {
		repl = true;
		scm_current_input_port = stdin;
	}
	else {
		repl = false;
		const char *file = argv[1];
		f = fopen(file, "r");
		if (f == NULL) {
			printf("cant open file %s\n", file);
			return 1;
		}
		scm_current_input_port = f;
	}

	while (1) {
		if (repl) {
			fputs("> ", stdout);
			fflush(stdout);
		}
		scm_obj_t obj = scm_read();
		if (scm_is_eof_object(obj)) { break; }
		else if (scm_is_error(obj)) { if (repl) continue; else break; }

#ifndef SCM_NO_EVAL
		obj = scm_eval(obj, scm_interaction_environment);
		if (scm_is_error(obj)) { if (repl) continue; else break; }
#endif

		/* Unspecified is returned by e.g. define. Dont print it. */
		if (!scm_is_unspecified(obj)) {
			scm_write(obj);
			putchar('\n');
		}
	}

	if (!repl) fclose(f);
	return 0;
}
