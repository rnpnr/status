/* See LICENSE for license details. */
#include <time.h>

#include "../status.h"
#include "../util.h"
#include "gettime.h"

size_t
gettime(struct Block *b)
{
	time_t t = time(NULL);
	return strftime(b->curstr, LEN(b->curstr), b->fmt, localtime(&t));
}
