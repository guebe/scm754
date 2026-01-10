/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scheme.h"
#include <stdio.h>

int main(void)
{
	scm_obj_t obj;

	while (1) {
		fputs("> ", stdout);
		fflush(stdout);

		obj = scm_read();
		if (scm_is_eof_object(obj)) { break; }

		scm_write(obj);
		putchar('\n');
	}

	return 0;
}
