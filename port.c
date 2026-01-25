/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scm754.h"

static int peek_buffer = EOF;
FILE *scm_current_input_port = NULL;

extern int scm_read_char(void)
{
	int c;

	if (peek_buffer == EOF) {
		c = getc(scm_current_input_port);
	} else {
		c = peek_buffer;
		peek_buffer = EOF;
	}

	return c;
}

extern int scm_peek_char(void)
{
	if (peek_buffer == EOF) {
		peek_buffer = getc(scm_current_input_port);
	}

	return peek_buffer;
}
