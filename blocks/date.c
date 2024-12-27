/* See LICENSE for license details. */
static BLOCK_UPDATE_FN(date_update)
{
	time_t t = time(NULL);
	strftime(buffer, sizeof(buffer), b->arg, localtime(&t));
	b->len = snprintf(b->data, sizeof(b->data), b->fmt, buffer);
}

static BLOCK_INIT_FN(date_init)
{
	date_update(b);
}
