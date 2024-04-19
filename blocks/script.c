/* See LICENSE for license details. */
static size_t
script(struct Block *b)
{
	FILE *fp;

	if ((fp = popen(b->arg, "r")) == NULL)
		die("popen()\n");

	if (fgets(buf, sizeof(buf), fp) != NULL)
		buf[strcspn(buf, "\n")] = 0;
	pclose(fp);

	return buf[0]? snprintf(b->curstr, LEN(b->curstr), b->fmt, buf) : 0;
}
