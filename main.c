#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX(V, H) ((V) > (H) ? (H) : (V))
#define OR(A, B)  ((A) ? (A) : (B))

/*
 * shut up, its for for readibility guis
 *
 * and anyways, programmars nowadays should stop pretending
 * that C's `char' is really capable of holding a character
 */
typedef unsigned char byte_t;

#define LINELEN    16

struct Style {
	char *s, *esc1, *esc2;
} styles[] = {
#define CTRL { "·", "\x1b[34m", "\x1b[34m" }
	[0]   = { "0", "\x1b[37m", "\x1b[37m" },
	[1]   = CTRL, [2]  = CTRL, [3]  = CTRL, [4]  = CTRL, [5]  = CTRL,
	[6]   = CTRL, [7]  = CTRL, [8]  = CTRL,
	[9]   = { "»", "\x1b[1;35m", "\x1b[35m" },
	[10]  = { "_", "\x1b[1;37m", "\x1b[1m"  },
	[11]  = CTRL, [12] = CTRL, [13] = CTRL, [14] = CTRL, [15] = CTRL,
	[16]  = CTRL, [17] = CTRL, [18] = CTRL, [19] = CTRL, [20] = CTRL,
	[21]  = CTRL, [22] = CTRL, [23] = CTRL, [24] = CTRL, [25] = CTRL,
	[26]  = CTRL, [27] = CTRL, [28] = CTRL, [29] = CTRL, [30] = CTRL,
	[31]  = CTRL,
	[127] = { "«", "\x1b[1;31m", "\x1b[31m" },
#undef CTRL
};

static void
display(byte_t *buf, size_t sz, size_t offset)
{
	printf("\x1b[37m%08zx\x1b[m    ", offset);

	for (byte_t *p = buf; (size_t)(p - buf) < sz; ++p) {
		if ((p - buf) == (LINELEN / 2))
			printf(" ");

		printf("%s%02hx\x1b[m ",
			OR(styles[*p].esc2, ""), *p);
	}

	if (LINELEN - sz > 0)
		printf("%*s", (int)((LINELEN - sz) * 3 + 1), "");
	printf("   │");

	for (byte_t *p = buf; (size_t)(p - buf) < sz; ++p) {
		if (styles[*p].s != NULL) {
			printf("%s%s\x1b[m", OR(styles[*p].esc1, ""),
					styles[*p].s);
		} else {
			putchar(*p);
		}
	}

	/*
	 * XXX: should we pad the ASCII column?
	 * Doing so may make it difficult to distinguish between
	 * End-of-input and spaces. On the other hand, padding
	 * makes the output look a bit, uh, ...cleaner, I guess?
	 */

	printf("│\n");
}

static void
huxdemp(byte_t *inp, size_t sz, size_t offset)
{
	byte_t line[LINELEN];

	for (byte_t *p = inp; (size_t)(p - inp) < sz; p += LINELEN) {
		memset(line, 0x0, LINELEN);

		byte_t cpyd = MAX(LINELEN, sz - (p - inp));
		memcpy(line, p, cpyd);
		display(line, cpyd, offset);
	}
}

int
main(void)
{
	size_t offset = 0;
	byte_t buf[4096];

	for (size_t r = 1; r > 0; huxdemp(buf, r, offset)) {
		memset(buf, 0x0, sizeof(buf));
		r = read(STDIN_FILENO, buf, sizeof(buf));
		offset += r;
	}
}
