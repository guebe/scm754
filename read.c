/* (c) guenter.ebermann@htl-hl.ac.at
 * Tiny Scheme Interpreter
 */
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "read.h"

#if 0
#define CLOS 6U /* closure - idx to cell with pair variables . body in env - a user defined function */
#define PRIM 7U /* primitive - idx to func ptr table - special forms - control evaluation - the primitive itself (callee) evaluates its arguments according to special rules */
#define PROC 8U /* procedure - idx to func ptr table - builtins - eval args then apply procedure - the caller already evaluated all arguments - the callee takes already evaluated arguments */
#define IS_BOX(d) (((d) & 0xfff8000000000000ULL) == 0xfff8000000000000ULL)
/* symbol table: list of strings key = symbol index (a b c) (cons a (cons b (cons c '()))) */
double symbols;
/* environments: list of list of pairs (symbol index . value|procedure) (((a . 3) (b . proc)) ((a . 5) (b . 8)) */
double env;
#endif

#define SCM_NIL  0U /* '() - the empty list */
#define SCM_BOOLEAN 1U /* LSB: 0 ... #f, 1 ... #t */
#define SCM_SYMBOL 2U /* index to len/8 cells */
#define SCM_STRING 3U /* index to len/8 cells */
#define SCM_CHARACTER 4U /* LSB: character */
#define SCM_CONS 5U /* cons: index to two cells */

#define SCM_TOKEN_SIZE 128U /* size of buffer in bytes used to read tokens */
#define SCM_CELL_NUM 8192U /* number of cells */
#define SCM_VALUE_MASK 0xffffffffffffULL

static double scm_cell[SCM_CELL_NUM];
static size_t scm_i = 0;

static inline double scm_box(unsigned int tag, uint64_t value)
{
	double d;
       	value = ((0xfff8ULL | (((uint64_t)tag) & 0x7ULL)) << 48U)
	       	| (value & SCM_VALUE_MASK);
       	memcpy(&d, &value, sizeof(d));
	return d;
}

static inline uint64_t scm_unbox(double d)
{
       	uint64_t u;
       	memcpy(&u, &(d), sizeof(u));
       	return u;
}

static inline double scm_cons(double a, double b)
{
	size_t tmp, i;

	tmp = i = scm_i;
	if (i + 2 >= SCM_CELL_NUM) errx(EXIT_FAILURE, "out of memory\n");
	scm_cell[i++] = a;
	scm_cell[i++] = b;
	scm_i = i;
	return scm_box(SCM_CONS, tmp);
}

static inline void scm_set_cdr(double a, double b)
{
	uint64_t i = scm_unbox(a) & SCM_VALUE_MASK;
	scm_cell[i+1] = b;
}

/* R7RS, section 7.1.1, Lexical structure */
static inline int scm_line_ending(int c) { return c == '\r' || c == '\n'; }
static inline int scm_whitespace(int c) { return c == ' ' || c == '\t' || scm_line_ending(c); }
static inline int scm_delimiter(int c) { return scm_whitespace(c) || c == '|' || c == '(' || c == ')' || c == '"' || c == ';'; }
static inline int scm_digit(int c) { return c >= '0' && c <= '9'; }
static inline int scm_letter(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static inline int scm_special_initial(int c) { return c == '!' || c == '$' || c == '%' || c == '&' || c == '*' || c == '/' || c == ':' || c == '<' || c == '=' || c == '>' || c == '?' || c == '^' || c == '_' || c == '~'; }
static inline int scm_initial(int c) { return scm_letter(c) || scm_special_initial(c); }

static int scm_char_buffer = EOF;

static inline int scm_getc(void)
{
	if (scm_char_buffer != EOF) {
	       	int c = scm_char_buffer;
	       	scm_char_buffer = EOF;
		return c;
	}
	return getchar();
}

static inline void scm_ungetc(int c)
{
	scm_char_buffer = c;
}

static void scm_scan_token(char *buf, size_t size)
{
	size_t i = 0;
	int c;

	while (((c = scm_getc()) != EOF) && !scm_delimiter(c) && (i < size))
		buf[i++] = c;

	if (i >= size)
	       	errx(EXIT_FAILURE, "read: token too long");

	buf[i] = 0;
	scm_ungetc(c);
}

static double scm_strtol(const char *buf, int base)
{
	char *end;
	errno = 0;
	long v = strtol(buf, &end, base);

	if (end == buf)	errx(EXIT_FAILURE, "read: nothing parsed");
	if (*end != '\0') errx(EXIT_FAILURE, "read: trailing garbage %s", buf);
	if (errno != 0)	err(EXIT_FAILURE, "read");

	return v;
}

static double scm_strtod(const char *buf)
{
	char *end;
	errno = 0;
	double d = strtod(buf, &end);

	if (end == buf)	errx(EXIT_FAILURE, "read: nothing parsed");
	if (*end != '\0') errx(EXIT_FAILURE, "read: trailing garbage %s", buf);
	if (errno != 0) err(EXIT_FAILURE, "read");

	return d;
}

static int scm_skip_whitespace(void)
{
	int c, c1;
	while (1) {
		c = scm_getc();
		if (scm_whitespace(c))
			continue;
		else if (c == ';')
       			while (((c1 = scm_getc()) != EOF) && !scm_line_ending(c));
		else
			return c;
	}
}

static double scm_read_boolean(int c)
{
	int c1;
       
	c1 = scm_getc();
	if (c1 == EOF || scm_delimiter(c1)) {
		scm_ungetc(c1);
		if (c == 't')
			return scm_box(SCM_BOOLEAN, 1);
		else if (c == 'f')
			return scm_box(SCM_BOOLEAN, 0);
	}

	errx(EXIT_FAILURE, "read: unexpected #%c%c", c, c1);
}

static double scm_read_character(void)
{
	int c, c1;

	c = scm_getc();
	if (c == EOF)
		errx(EXIT_FAILURE, "read: unexpected EOF after #\\");

	c1 = scm_getc();
	if (c1 == EOF || scm_delimiter(c1))
		return scm_box(SCM_CHARACTER, c);

	errx(EXIT_FAILURE, "read: unexpected #\\%c%c", c, c1);
}

static double scm_read_number_radix(int radix)
{
	char buf[SCM_TOKEN_SIZE];
	scm_scan_token(buf, sizeof(buf));
	if (buf[0] == 0)
		errx(EXIT_FAILURE, "read: unexpected number format #<radix><delimiter>");

	if (radix == 10)
		return scm_strtod(buf);
	else	
		return scm_strtol(buf, radix);
}

static double scm_read_sharp(void)
{
	int c = scm_getc();

	if (c == 'f' || c == 't')
		return scm_read_boolean(c);
	else if (c == '\\')
		return scm_read_character();
	else if (c == 'b' || c == 'B')
		return scm_read_number_radix(2);
	else if (c == 'o' || c == 'O')
		return scm_read_number_radix(8);
	else if (c == 'd' || c == 'D')
		return scm_read_number_radix(10);
	else if (c == 'x' || c == 'X')
		return scm_read_number_radix(16);
	else
		errx(EXIT_FAILURE, "read: unexpected #%c", c);
}

static double scm_read_number_digit(char c)
{
	char buf[SCM_TOKEN_SIZE];

	buf[0] = c;
	scm_scan_token(buf + 1, sizeof(buf) - 1);

	return scm_strtod(buf);
}

static double scm_make_symbol(const char *buf)
{
	buf = buf;
	return scm_box(SCM_SYMBOL, 0);
}

static double scm_read_sexp(void)
{
	int c;
	double tmp, head, last, tail, nil;
	
  	tmp = scm_read_expr();
	nil = scm_box(SCM_NIL, 0);
	head = last = scm_cons(tmp, nil);

	while (1) {
		c = scm_skip_whitespace();

		if (c == ')')
			return head;
		else if (c == '.') {
			scm_set_cdr(last, scm_read_expr());
			c = scm_skip_whitespace();
			if (c != ')')
				errx(EXIT_FAILURE, "read: missing )");
			return head;
		}
		else {
			scm_ungetc(c);
			tmp = scm_read_expr();
			tail = scm_cons(tmp, nil);
			scm_set_cdr(last, tail);
			last = tail;
		}
	}
}

static double scm_read_string(void)
{
	size_t start, i;
	int c;
       
	start = scm_i;
	i = start * 8U;

	while (((c = scm_getc()) != EOF) && (c != '"') && (((i + 1)/ 8) < SCM_CELL_NUM))
		((uint8_t*)scm_cell)[i++] = c;

	if (((i + 1)/ 8) >= SCM_CELL_NUM)
	       	errx(EXIT_FAILURE, "read: string too long");

	((uint8_t*)scm_cell)[i] = 0;
	return scm_box(SCM_STRING, start);
}

static double scm_read_quote(void)
{
	return scm_cons(0, 0);
}
		       	
static double scm_read_symbol(char c)
{
	return scm_box(SCM_SYMBOL, c);
} 

static double scm_read_sign(char c)
{
	char buf[SCM_TOKEN_SIZE];
	buf[0] = c;
	scm_scan_token(buf + 1, sizeof(buf) - 1);
	if (buf[1] == 0)
		return scm_make_symbol(buf);
	else if (scm_digit(buf[1]))
		return scm_strtod(buf);
	else
		errx(EXIT_FAILURE, "read: unexpected %s", buf);
}

double scm_read_expr(void)
{
	int c;

	c = scm_skip_whitespace();

	if (c == '(')
	       	return scm_read_sexp();
	else if (c == '"')
	       	return scm_read_string();
	else if (c == '\'')
	       	return scm_read_quote();
	else if (c == '#')
	       	return scm_read_sharp();
	else if (c == '+' || c == '-')
		return scm_read_sign(c);
	else if (scm_digit(c))
	       	return scm_read_number_digit(c);
	else if (scm_initial(c))
	       	return scm_read_symbol(c);
	else if (c == EOF)
		exit(EXIT_SUCCESS);
	else
	       	errx(EXIT_FAILURE, "read: unexpected %c", c);
}
