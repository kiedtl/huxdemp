#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static size_t _buf_len = 0;

static _Bool
parse_int(int *x, char *s, char **e, _Bool add, uint8_t *buf)
{
	size_t base;
	if (!strncmp(s, "0x", 2) || !strncmp(s, "U+", 2)) {
		base = 16;
		s += 2;
	} else if (!strncmp(s, "0o", 2)) {
		base = 8;
		s += 2;
	} else if (!strncmp(s, "0b", 2)) {
		base = 2;
		s += 2;
	} else {
		base = 10;
	}

	*x = strtol(s, e, base);
	_Bool ok = *e != s;

	/* HACK: the add parameter controls whether a parsed integer is
	 * added to the entries if it succeeds in parsing it.
	 *
	 * The reason it is needed is because parsed_int is used in two
	 * places: 1) in expand_range (where we *want* successfully parse
	 * integers to be added to the entries) and 2) in parse_range (where
	 * we *don't want* successfully parsed integers to be added to the
	 * entries.
	 */

	if (ok && add) buf[_buf_len] = *x, ++_buf_len;
	return ok;
}


static _Bool
parse_range(char *s, char **e, uint8_t *buf)
{
	int x = 0, y = 0;
	char *ee;
	char *start = s;

	/* try to parse left-hand side of range */
	if (!parse_int(&x, s, &ee, false, buf))
		return false;
	s = ee;

	/* check if this is really a range, or just
	 * a single integer */
	if (*s != '-') {
		e = &start;
		return false;
	} else {
		++s;
	}
	
	/* try to parse right-hand side of range */
	if (!parse_int(&y, s, e, false, buf))
		return false;

	/* check if left-hand size is greater than
	 * right-hand side of range */
	if (y < x) return false;

	/* copy onto accumulator */
	for (size_t i = x; i <= (size_t)y; ++i)
		buf[_buf_len] = i, ++_buf_len;
	return true;
}

static ssize_t
expand_range(char *s, uint8_t *buf)
{
	_buf_len = 0;
	int x = 0;
	char **e = &s;

	for (;;) {
		while (isspace(*s)) ++s;

		/*
		 * try to parse input as a range, and fall back
		 * to parsing input as a single integer if that
		 * failed.
		 * if both failed, it's probably a syntax error.
		 */
		if (!parse_range(s, e, buf)) {
			if (!parse_int(&x, s, e, true, buf)) {
				break;
			}
		}
		s = *e;
		
		while (isspace(*s)) ++s;
		if (strlen(s) == 0) return _buf_len;

		/* check if there's something more to parse */
		if ((*s) == ',') {
			++s;
			continue;
		}

		break;
	}

	/* if we broke out of the main loop then a syntax
	 * error must have occurred */
	return -1;
}
