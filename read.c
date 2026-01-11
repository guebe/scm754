/* (c) guenter.ebermann@htl-hl.ac.at */

#include "scheme.h"
#include <stdio.h>

#define SCM_TOKEN_SIZE  128
#define SCM_STRING_SIZE 512
#define SCM_READ_DEPTH 8192

static size_t read_depth;

/* R7RS, section 7.1.1, Lexical structure */
static inline _Bool is_line_ending(int c) { return c == '\r' || c == '\n'; }
static inline _Bool is_whitespace(int c) { return c == ' ' || c == '\t' || is_line_ending(c); }
static inline _Bool is_delimiter(int c) { return is_whitespace(c) || c == '|' || c == '(' || c == ')' || c == '"' || c == ';'; }
static inline _Bool is_digit(int c) { return c >= '0' && c <= '9'; }
static inline _Bool is_letter(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static inline _Bool is_special_initial(int c) { return c == '!' || c == '$' || c == '%' || c == '&' || c == '*' || c == '/' || c == ':' || c == '<' || c == '=' || c == '>' || c == '?' || c == '^' || c == '_' || c == '~'; }
static inline _Bool is_initial(int c) { return is_letter(c) || is_special_initial(c); }
static inline _Bool is_explicit_sign(int c) { return c == '+' || c == '-'; }

static void skip_comment(void)
{
	int c;
	while (((c = scm_read_char()) != EOF) && !is_line_ending(c));
}

static scm_obj_t read_boolean(int c)
{
	int c1;

	c1 = scm_peek_char();
	if (c1 == EOF || is_delimiter(c1)) {
		if (c == 't') return scm_true();
		else if (c == 'f') return scm_false();
	}

	return scm_error("read_boolean: unexpected #%c%c", c, c1);
}

static scm_obj_t read_char(void)
{
	int c, c1;

	c = scm_read_char();
	if (c == EOF)
		return scm_error("read_char: unexpected EOF after #\\");

	c1 = scm_peek_char();
	if (c1 == EOF || is_delimiter(c1))
		return scm_char((char)c);

	return scm_error("read_char: unexpected #\\%c%c", c, c1);
}

__attribute__((warn_unused_result))
static ssize_t scan_token(char *buf, size_t size)
{
	ssize_t i = 0, ssize = (ssize_t)size;
	int c;

	while (((c = scm_peek_char()) != EOF) && !is_delimiter(c) && (i < ssize)) {
		scm_read_char();
		buf[i++] = (char)c;
	}

	if (i >= ssize) return -1;

	buf[i] = '\0';
	return i;
}

static scm_obj_t read_number_radix(int radix)
{
	char buf[SCM_TOKEN_SIZE];

	if (scan_token(buf, sizeof buf) <= 0)
		return scm_error("read_number_radix: scan error");

	return scm_string_to_number(buf, radix);
}

static scm_obj_t read_sharp(void)
{
	int c = scm_read_char();

	if (c == 'f' || c == 't')
		return read_boolean(c);
	else if (c == '\\')
		return read_char();
	else if (c == 'b' || c == 'B')
		return read_number_radix(2);
	else if (c == 'o' || c == 'O')
		return read_number_radix(8);
	else if (c == 'd' || c == 'D')
		return read_number_radix(10);
	else if (c == 'x' || c == 'X')
		return read_number_radix(16);
	else
		return scm_error("read_sharp: unexpected #%c", c);
}

static scm_obj_t read_number(char c)
{
	char buf[SCM_TOKEN_SIZE];

	buf[0] = c;
	if (scan_token(buf + 1, sizeof buf - 1) < 0)
		return scm_error("read_number: scan error");

	return scm_string_to_number(buf, 0);
}

static scm_obj_t read_symbol(char c)
{
	char buf[SCM_TOKEN_SIZE];
	ssize_t len;
	scm_obj_t obj;

	buf[0] = c;
	if ((len = scan_token(buf + 1, sizeof buf - 1)) < 0)
		return scm_error("read_symbol: scan error");

	obj = scm_string(buf, (size_t)len+1);
	if (scm_is_error_object(obj)) return obj;

	return scm_string_to_symbol(obj);
}

static scm_obj_t read_symbol_or_number(char c)
{
	char buf[SCM_TOKEN_SIZE];
	ssize_t len;
	scm_obj_t obj;

	buf[0] = c;
	if ((len = scan_token(buf + 1, sizeof buf - 1)) < 0)
		return scm_error("read_symbol_or_number: scan error");

	obj = scm_string_to_number(buf, 0);
	if (scm_boolean_value(obj)) return obj;

	obj = scm_string(buf, (size_t)len+1);
	if (scm_is_error_object(obj)) return obj;
	       
	return scm_string_to_symbol(obj);
}

static scm_obj_t read_string(void)
{
	char buf[SCM_STRING_SIZE];
	size_t n = 0;
	int c;

	while (((c = scm_read_char()) != EOF) && (c != '"') && (n < sizeof buf))
		buf[n++] = (char)c;

	if (n >= sizeof buf) return scm_error("read_string: string too long");

	buf[n] = 0;

	return scm_string(buf, n);
}

static scm_obj_t read(_Bool dot_ok, _Bool rparen_ok, _Bool eof_ok);
static scm_obj_t read_list(void)
{
	scm_obj_t obj, head, last;

	obj = read(0, 1, 0);
	if (scm_is_error_object(obj)) return obj;
	if (scm_is_rparen(obj)) return scm_empty_list();

	head = last = scm_cons(obj, scm_empty_list());
	if (scm_is_error_object(head)) return head;

	while (1) {
		obj = read(1, 1, 0);
		if (scm_is_error_object(obj)) return obj;
		if (scm_is_rparen(obj)) return head;
		if (scm_is_dot(obj)) {
			/* Read dotted pair */
			obj = read(0, 0, 0);
			if (scm_is_error_object(obj)) return obj;

			scm_set_cdr(last, obj);

			/* After dot, next must be rparen */
			obj = read(0, 1, 0);
			if (scm_is_error_object(obj)) return obj;
			if (!scm_is_rparen(obj)) return scm_error("read: closing rparen ) missing");
			return head;
		}
		else {
			/* Normal cons */
			obj = scm_cons(obj, scm_empty_list());
			if (scm_is_error_object(obj)) return obj;

			scm_set_cdr(last, obj);
			last = obj;
		}
	}
}

static scm_obj_t read_quote(void)
{
	scm_obj_t quote, datum, args;

	quote = scm_string("quote", 5);
	if (scm_is_error_object(quote)) return quote;

	datum = read(0, 0, 0);
	if (scm_is_error_object(datum)) return datum;

	args = scm_cons(datum, scm_empty_list());
	if (scm_is_error_object(args)) return args;

	return scm_cons(scm_string_to_symbol(quote), args);
}

static scm_obj_t read(_Bool dot_ok, _Bool rparen_ok, _Bool eof_ok)
{
	int c;

	if (++read_depth > SCM_READ_DEPTH)
		return scm_error("read: maximum recursion depth exceeded");

	while (1) {
		c = scm_read_char();

		if (is_whitespace(c))
			continue;
		else if (c == ';')
			skip_comment();
		else if (c == EOF)
			return eof_ok ? scm_eof_object() : scm_error("read: unexpected end-of-file");
		else if (c == '(')
			return read_list();
		else if (c == ')')
			return rparen_ok ? scm_rparen() : scm_error("read: too many )");
		else if (c == '.')
			return dot_ok ? scm_dot() : scm_error("read: unexpected dot (.)");
		else if (c == '"')
			return read_string();
		else if (c == '\'')
			return read_quote();
		else if (c == '#')
			return read_sharp();
		else if (is_digit(c))
			return read_number((char)c);
		else if (is_initial(c))
			return read_symbol((char)c);
		else if (is_explicit_sign(c))		
			return read_symbol_or_number((char)c);
		else
			return scm_error("read: unexpected %c", c);
	}
}

extern scm_obj_t scm_read(void)
{
	read_depth = 0;
	return read(0, 0, 1);
}
