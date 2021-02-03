#include <err.h>    /* for warn */
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* for getenv */
#include <unistd.h> /* for isatty, STDOUT_FILENO */

#include "arg.h"
#include "tables.c"
#include "utf8.c"

#define MAX(V, H) ((V) > (H) ? (H) : (V))
#define OR(A, B)  ((A) ? (A) : (B))

/*
 * shut up, its for for readibility guis
 *
 * and anyways, programmars nowidays should stahp pretending
 * like C's `char' is really capable of holding a character
 */
typedef unsigned char byte_t;

#define LINELEN    16

struct {
	char **table;
	_Bool ctrls, utf8;

	enum {
		C_AUTO, C_ALWAYS, C_NEVER,
	} color;

	/* not directly set by the user. this is determined after
	 * the user sets `usecolor' */
	_Bool _color;
} options;

/* Keep track of what UTF8 codepoint we're printing. This
 * allows us to highlight bytes that belong to the same Unicode
 * codepoint.
 *
 * This is reset after each file, for obvious reasons.
 *
 * { <offset>, <length> } (the offset is the position of the
 * first byte in the encoded UTF8, the length is the number of
 * bytes contained therin) */
static ssize_t utf8_state[] = { -1, -1 };

/* The colors used to highlight bytes that belong to the same
 * codepoint. */
static const uint8_t utf8bg[] = {
	0, 230, 229, 228, 227, 226, 189
};

static inline void
_utf8state(ssize_t offset, byte_t _char)
{
	if (utf8_state[0] == -1 || utf8_state[0]+utf8_state[1] < offset) {
		utf8_state[0] = offset;
		utf8_state[1] = utf8_char_length(_char) - 1;
	}
}

static inline char *
_format_char(byte_t b)
{
	static char chbuf[2] = {0};

	if (options.ctrls && t_cntrls[b])
		return t_cntrls[b];

	if (options.table && options.table[b])
		return options.table[b];

	chbuf[0] = isprint(b) ? b : '.';
	return (char *)&chbuf;
}

/*
 * XXX: should we pad the ASCII column?
 * Doing so may make it difficult to distinguish between
 * End-of-input and spaces. On the other hand, padding
 * makes the output look a bit, uh, ...cleaner, I guess?
 */

static void
display(byte_t *buf, size_t sz, size_t offset)
{
	printf("\x1b[37m%08zx\x1b[m    ", offset);

	for (size_t off = offset, i = 0; i < sz; ++i, ++off) {
		_utf8state((ssize_t)off, buf[i]);

		if (i == (LINELEN / 2))
			printf(" ");

		printf("\x1b[48;5;%dm%s%02hx",
			options.utf8 ? utf8bg[utf8_state[1]] : 0,
			OR(styles[buf[i]].esc2, ""), buf[i]);

		if (utf8_state[0] + utf8_state[1] <= (ssize_t)off)
			printf("\x1b[m ");
		else
			printf("\x1b[97m\x1b[22m ");
	}

	printf("\x1b[m");
	if (LINELEN - sz > 0) {
		printf("%*s", (int)(LINELEN - sz) * 3, "");
		if (sz <= (LINELEN / 2))
			printf(" ");
	}
	printf("   │");

	for (size_t off = offset, i = 0; i < sz; ++i, ++off)
		printf("%s%s\x1b[m", OR(styles[buf[i]].esc1, ""),
			_format_char(buf[i]));

	printf("│\n");
}

static void
display_nocolor(byte_t *buf, size_t sz, size_t offset)
{
	printf("%08zx    ", offset);

	for (size_t i = 0; i < sz; ++i) {
		if (i == (LINELEN / 2))
			printf(" ");
		printf("%02hx ", buf[i]);
	}

	if (LINELEN - sz > 0) {
		printf("%*s", (int)(LINELEN - sz) * 3, "");
		if (sz <= (LINELEN / 2))
			printf(" ");
	}

	printf("   |");
	for (size_t i = 0; i < sz; ++i)
		printf("%s", _format_char(buf[i]));
	printf("|\n");
}


static void
splitinput(byte_t *inp, size_t sz)
{
	static size_t offset = 0;
	byte_t line[LINELEN];

	void (*func)(byte_t *, size_t, size_t);
	func = options._color ? display : display_nocolor;

	for (size_t i = 0; i < sz; i += LINELEN) {
		memset(line, 0x0, LINELEN);

		byte_t cpyd = MAX(LINELEN, sz - i);
		memcpy(line, &inp[i], cpyd);
		(func)(line, cpyd, offset);
		offset += cpyd;
	}
}

static void
huxdemp(char *path)
{
	utf8_state[0] = utf8_state[1] = -1;

	byte_t buf[32768];
	FILE *fp = !strcmp(path, "-") ? stdin : fopen(path, "r");

	if (fp == NULL) {
		warn("\"%s\"", path);
		return;
	}

	for (size_t r = 1; r > 0; splitinput(buf, r))
		r = fread(buf, sizeof(byte_t), sizeof(buf), fp);

	if (strcmp(path, "-"))
		fclose(fp);

	printf("\n");
}

static _Bool
_decide_color(void)
{
	if (options.color == C_ALWAYS)
		return true;
	if (options.color == C_NEVER)
		return false;

	if (!isatty(STDOUT_FILENO))
		return false;

	char *env_NOCOLOR = getenv("NO_COLOR");
	char *env_TERM = getenv("TERM");

	if (env_NOCOLOR)
		return false;

	if (!env_TERM || !strcmp(env_TERM, "dumb"))
		return false;

	return true;
}

static _Noreturn void
_usage(char *argv0)
{
	printf("Usage: %s [-hV]\n", argv0);
	printf("       %s [-cu] [-t table] [-C color?] [FILE]...\n", argv0);
	printf("\n");
	printf("Flags:\n");
	printf("    -c  Use Unicode glyphs to display the lower control\n");
	printf("        chars (0 to 31). E.g. ␀ for NUL, ␖ for SYN (0x16), &c\n");
	printf("    -u  Highlight sets of bytes that 'belong' to the same UTF-8\n");
	printf("        encoded Unicode character.\n");
	printf("    -h  Print this help message and exit.\n");
	printf("    -V  Print hxd's version and exit.\n");
	printf("\n");
	printf("Options:\n");
	printf("    -t  What 'table' or style to use.\n");
	printf("        Possible values: `default', `cp437', or `classic'.\n");
	printf("    -C  When to use fancy terminal formatting.\n");
	printf("        Possible values: `auto', `always', `never'.\n");
	printf("\n");
	printf("Arguments are processed in the same way that cat(1) does: any\n");
	printf("arguments are treated as files and read, a lone \"-\" causes hxd\n");
	printf("to read from standard input, &c.\n");
	printf("\n");
	printf("See the manpage hxd(1) for more documentation.\n");
	_Exit(0);
}

int
main(int argc, char *argv[])
{
	options.table = (char **)&t_default;
	options.ctrls = options.utf8 = false;
	options.color = C_AUTO;

	char *argv0 = argv[0], *optarg;

	ARGBEGIN {
	break; case 'c':
		options.ctrls = !options.ctrls;
	break; case 'u':
		options.utf8  = !options.utf8;
	break; case 't':
		optarg = EARGF(_usage(argv0));
		if (!strncmp(optarg, "cp", 2))
			options.table = (char **)&t_cp437;
		else if (!strncmp(optarg, "de", 2))
			options.table = (char **)&t_default;
		else if (!strncmp(optarg, "cl", 2))
			options.table = NULL;
		else
			_usage(argv[0]);
	break; case 'C':
		optarg = EARGF(_usage(argv0));
		if (!strncmp(optarg, "au", 2))
			options.color = C_AUTO;
		else if (!strncmp(optarg, "al", 2))
			options.color = C_ALWAYS;
		else if (!strncmp(optarg, "ne", 2))
			options.color = C_NEVER;
		else
			_usage(argv[0]);
	break; case 'v': case 'V':
		printf("hxd v"VERSION"\n");
		return 0;
	break; case 'h': case '?': default:
		_usage(argv[0]);
	} ARGEND

	options._color = _decide_color();

	if (!argc) {
		huxdemp("-");
	} else {
		for (; *argv; --argc, ++argv)
			huxdemp(*argv);
	}

	return 0;
}
