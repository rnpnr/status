/* See LICENSE for license details. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "status.h"
#include "util.h"

void
die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(1);
}

int
pscanf(const char *path, const char *fmt, ...)
{
	FILE *fp;
	va_list ap;
	int ret;

	if (!(fp = fopen(path, "r")))
		return -1;

	va_start(ap, fmt);
	ret = vfscanf(fp, fmt, ap);
	va_end(ap);
	fclose(fp);

	return (ret == EOF) ? -1 : ret;
}
