/* See LICENSE for license details. */
#include <stdio.h>
#include <time.h>

#include "../status.h"
#include "../util.h"
#include "date.h"

size_t
date(struct Block *b)
{
	time_t t = time(NULL);
	strftime(buf, sizeof(buf), b->arg, localtime(&t));

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
