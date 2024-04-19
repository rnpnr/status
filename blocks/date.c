/* See LICENSE for license details. */
static size_t
date(struct Block *b)
{
	time_t t = time(NULL);
	strftime(buf, sizeof(buf), b->arg, localtime(&t));

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
