#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tables.h"
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

	/*
	 * XXX: should we pad the ASCII column?
	 * Doing so may make it difficult to distinguish between
	 * End-of-input and spaces. On the other hand, padding
	 * makes the output look a bit, uh, ...cleaner, I guess?
	 */

	printf("│\n");
}

static void
splitinput(byte_t *inp, size_t sz)
{
	static size_t offset = 0;
	byte_t line[LINELEN];

	for (size_t i = 0; i < sz; i += LINELEN) {
		memset(line, 0x0, LINELEN);

		byte_t cpyd = MAX(LINELEN, sz - i);
		memcpy(line, &inp[i], cpyd);
		display(line, cpyd, offset);
		offset += cpyd;
	}
}

static void
huxdemp(char *path)
{
	utf8_state[0] = utf8_state[1] = -1;

	byte_t buf[32768];
	FILE *fp = !strcmp(path, "-") ? stdin : fopen(path, "r");

	for (size_t r = 1; r > 0; splitinput(buf, r))
		r = fread(buf, sizeof(byte_t), sizeof(buf), fp);

	if (strcmp(path, "-"))
		fclose(fp);

	printf("\n");
}

int
main(int argc, char **argv)
{
	options.table = (char **)&t_default;
	options.ctrls = options.utf8 = false;

	ssize_t opt;
	while ((opt = getopt(argc, argv, "cut:")) != -1) {
		switch(opt) {
		break; case 'u':
			options.utf8  = !options.utf8;
		break; case 'c':
			options.ctrls = !options.ctrls;
		break; case 't':
			if (!strncmp(optarg, "cp", 2)) {
				options.table = (char **)&t_cp437;
			} else if (!strncmp(optarg, "de", 2)) {
				options.table = (char **)&t_default;
			} else if (!strncmp(optarg, "cl", 2)) {
				options.table = NULL;
			} else {
				fprintf(stderr, "Invalid option to -t\n");
				return 1;
			}
		break; case '?':
			fprintf(stderr, "Usage: TODO\n");
			return 1;
		}
	};

	if (argv[optind] == NULL)
		huxdemp("-");

	for (size_t i = optind; i < (size_t)argc; ++i) {
		if (argv[i] != NULL)
			huxdemp(argv[i]);
	}

	return 0;
}
