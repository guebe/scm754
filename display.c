/* (c) guenter.ebermann@htl-hl.ac.at
 * Tiny Scheme Interpreter
 *
 * display procedure
 */

static void scm_display_list(double obj)
{
	unsigned int tag;

	putchar('(');
	while (1) {
		scm_display(scm_car(obj));
		obj = scm_cdr(obj);
		tag = scm_get_tag(scm_unbox(obj));
		
		if (tag == SCM_EMPTY_LIST)
			break;
		else if (tag == SCM_PAIR)
			putchar(' ');
		else {
			fputs(" . ", stdout);
			scm_display(obj);
			break;
		}
	}
	putchar(')');
}

extern void scm_display(double obj)
{
	uint64_t x, value;
	unsigned int tag;

	x = scm_unbox(obj);
	tag = scm_get_tag(x);
	value = scm_value(x);

	switch (tag) {
	case SCM_EMPTY_LIST:
		fputs("()", stdout);
		break;
	case SCM_BOOLEAN:
		fputs(value ? "#t" : "#f", stdout);
		break;
	case SCM_SYMBOL:
		fputs((char *)&scm_cell[value], stdout);
		break;
	case SCM_STRING:
		putchar('\"');
		fputs((char *)&scm_cell[value], stdout);
		putchar('\"');
		break;
	case SCM_CHARACTER:
		fputs("#\\", stdout);
		putchar(value);
		break;
	case SCM_PAIR:
		scm_display_list(obj);
		break;
	default:
		printf("%.10lg", obj);
		break;
	}
}

extern void scm_newline(void)
{
	putchar('\n');
}

