#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

_Bool inblock = false;

static void
unu(char *fname, size_t lineno, char *line)
{
	if (!strcmp(line, "~~~")) {
		inblock = !inblock;
		if (inblock) {
			printf("#line %zu \"%s\"\n", lineno, fname);
			printf("\n");
		}
		return;
	} else if (inblock) {
		printf("%s\n", line);
	}
}

int
main(int argc, char **argv)
{
	char *fname = NULL;
	size_t lineno = 0;

	if (argc != 2) {
		fprintf(stderr, "usage: %s [filename] < input > output\n", argv[0]);
		return 1;
	}

	fname = argv[1];

	// ---

	char buf[4096] = {0}, *p = (char *)&buf;
	ssize_t ch = -1;

	while ((ch = fgetc(stdin)) != EOF) {
		switch (ch) {
		break; case '\n':
			++lineno, *p = '\0', unu(fname, lineno, buf);
			buf[0] = '\0', p = (char *)&buf;
		break; default:
			*p = ch, ++p;
		}
	}

	return 0;
}
