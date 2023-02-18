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

	if ((fp = popen(b->arg, "r")) == NULL)
		die("popen()\n");

	if (fgets(buf, sizeof(buf), fp) != NULL)
		buf[strcspn(buf, "\n")] = 0;
	pclose(fp);

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
