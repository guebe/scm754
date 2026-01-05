/* (c) guenter.ebermann@htl-hl.ac.at
 * Tiny Scheme Interpreter
 *
 * Recursive descent parser
 * exports the read procedure
 */

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

#define SCM_TOKEN_SIZE 128U /* size of buffer in bytes used to read tokens */
#define SCM_STRING_SIZE 512U /* size of buffer in bytes used to read strings */

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

static size_t scm_scan_token(char *buf, size_t size)
{
	size_t i = 0;
	int c;

	while (((c = scm_getc()) != EOF) && !scm_delimiter(c) && (i < size))
		buf[i++] = c;

	if (i >= size)
		errx(EXIT_FAILURE, "read: token too long");

	buf[i] = 0;
	scm_ungetc(c);
	return i;
}

static size_t scm_scan_string(char *buf, size_t size)
{
	size_t i = 0;
	int c;

	while (((c = scm_getc()) != EOF) && (c != '"') && (i < size))
		buf[i++] = c;

	if (i >= size)
		errx(EXIT_FAILURE, "read: string too long");

	buf[i] = 0;
	return i;
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

static void scm_skip_comment(void)
{
	int c;
	while (((c = scm_getc()) != EOF) && !scm_line_ending(c));
}

static int scm_skip_whitespace(void)
{
	int c;
	while (1) {
		c = scm_getc();
		if (scm_whitespace(c))
			continue;
		else if (c == ';')
			scm_skip_comment();
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
	size_t len;
	char buf[SCM_TOKEN_SIZE];
	len = scm_scan_token(buf, sizeof(buf));
	if (len == 1)
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

static double scm_make_symbol(const char *buf, size_t size)
{
	size_t idx;

	idx = scm_cell_put(buf, size);

	if (idx >= SCM_CELL_NUM)
		errx(EXIT_FAILURE, "read: symbol too long for cell");

	return scm_box(SCM_SYMBOL, idx);
}

static double scm_read_list(void)
{
	int c;
	double tmp, head, last, tail, nil;
	nil = scm_box(SCM_NIL, 0);

	c = scm_skip_whitespace();
	if (c == ')')
		return nil;

	scm_ungetc(c);
  	tmp = scm_read();
	head = last = scm_cons(tmp, nil);

	while (1) {
		c = scm_skip_whitespace();

		if (c == ')')
			return head;
		else if (c == '.') {
			scm_set_cdr(last, scm_read());
			c = scm_skip_whitespace();
			if (c != ')')
				errx(EXIT_FAILURE, "read: missing )");
			return head;
		}
		else {
			scm_ungetc(c);
			tmp = scm_read();
			tail = scm_cons(tmp, nil);
			scm_set_cdr(last, tail);
			last = tail;
		}
	}
}

static double scm_read_string(void)
{
	char buf[SCM_STRING_SIZE];
	size_t len, idx;

	len = scm_scan_string(buf, sizeof(buf));
	idx = scm_cell_put(buf, len);

	if (idx >= SCM_CELL_NUM)
		errx(EXIT_FAILURE, "read: string too long for cell");

	return scm_box(SCM_STRING, idx);
}

static double scm_read_quote(void)
{
	return scm_cons(0, 0);
}

static double scm_read_symbol(char c)
{
	char buf[SCM_TOKEN_SIZE];
	size_t len;

	buf[0] = c;
	len = scm_scan_token(buf + 1, sizeof(buf) - 1);

	return scm_make_symbol(buf, len+1);
}

static double scm_read_sign(char c)
{
	char buf[SCM_TOKEN_SIZE];
	size_t len;

	buf[0] = c;
	len = scm_scan_token(buf + 1, sizeof(buf) - 1);
	if (len == 0)
		return scm_make_symbol(buf, 2);
	else if (scm_digit(buf[1]))
		return scm_strtod(buf);
	else
		errx(EXIT_FAILURE, "read: unexpected %s", buf);
}

extern double scm_read(void)
{
	int c;

	while (1) {
		c = scm_getc();

		if (scm_whitespace(c))
			continue;
		else if (c == ';')
			scm_skip_comment();
		else if (c == '(')
			return scm_read_list();
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
}
