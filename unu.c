#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

_Bool inblock = false;

static void
unu(char *line)
{
	if (!strcmp(line, "~~~")) {
		inblock = !inblock;
		if (inblock)
			printf("\n");
		return;
	}

	if (inblock)
		printf("%s\n", line);
}

int
main(void)
{
	char buf[4096] = {0}, *p = (char *)&buf;
	ssize_t ch = -1;

	while ((ch = fgetc(stdin)) != EOF) {
		switch (ch) {
		break; case '\n':
			*p = '\0', unu(buf);
			buf[0] = '\0', p = (char *)&buf;
		break; default:
			*p = ch, ++p;
		}
	}

	return 0;
}
