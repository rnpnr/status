/* See LICENSE for license details. */
#include <stdio.h>
#include <string.h>

#include "../status.h"
#include "../util.h"
#include "script.h"

size_t
script(struct Block *b)
{
	FILE *fp;

	if ((fp = popen(b->u.s, "r")) == NULL)
		die("popen()\n");

	fgets(buf, sizeof(buf), fp);
	pclose(fp);

	if (buf[strlen(buf) - 1] == '\n')
		buf[strlen(buf) - 1] = 0;

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
