/*
 * Based on ~sircmpwn's koio (tool/main.c).
 * https://git.sr.ht/~sircmpwn/koio
 */

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s input… > output.c\n", argv[0]);
		return 1;
	}

	printf("struct {\n"
		"	char *name;\n"
		"	char *path;\n"
		"	char *data;\n"
		"} embedded_files[] = {\n");

	for (size_t i = 1; i < (size_t)argc; ++i) {
		char *path = argv[i];
		char *name = argv[i];

		char *colon = strchr(argv[i], ':');
		if (colon) {
			*colon = '\0';
			name = colon + 1;
		}

		FILE *in = fopen(path, "r");
		if (!in) {
			err(1, "%s: Cannot open", path);
		}

		printf("\t{\n\t\t\"%s\", \"@%s\",", name, path);

		int cols = 1;
		int c;
		while ((c = fgetc(in)) != EOF) {
			if (cols == 1) {
				printf("\n\t\t\"");
				cols += 16;
			}
			switch (c) {
			case '\\':
				cols += printf("\\\\");
				break;
			case '\"':
				cols += printf("\\\"");
				break;
			case '\n':
				cols += printf("\\n");
				break;
			case '\t':
				cols += printf("\\t");
				break;
			default:
				if (isprint(c)) {
					cols += printf("%c", c);
				} else {
					cols += printf("\\x%02X", c);
				}
				break;
			}

			if (cols >= 90) {
				cols = 1;
				printf("\"");
			}
		}

		if (cols != 1) {
			printf("\"");
		}
		printf("\n\t},\n");
	}

	printf("};\n");

	return 0;
}
