/* See LICENSE for license details. */
struct date_arg {
	char *fmt;      /* Time Format String (ex: %R) */
	f32   interval; /* Update Interval [s] */
};

static BLOCK_UPDATE_FN(date_update)
{
	struct date_arg *da = b->arg;

	b32 result = timer_update(b->user_data, da->interval, dt);
	if (result) {
		time_t t = time(NULL);
		strftime(buffer, sizeof(buffer), da->fmt, localtime(&t));
		b->len = snprintf(b->data, sizeof(b->data), b->fmt, buffer);
	}

	return result;
}

static BLOCK_INIT_FN(date_init)
{
	b->user_data = alloc(a, f32, 1);
	date_update(b, 1);
}
