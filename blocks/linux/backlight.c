/* See LICENSE for license details. */
struct linux_backlight_data {i64 max_brightness; char *brightness_path;};

static BLOCK_UPDATE_FN(backlight_update)
{
	if (dt > 0)
		return 0;

	struct linux_backlight_data *lbd = b->user_data;
	i64 current;
	if (pscanf(lbd->brightness_path, "%ld", &current) != 1)
		current = 0;

	f32 percent = 100 * current / (f32)lbd->max_brightness + 0.5;
	i64 len = snprintf(buffer, sizeof(buffer), "%d%%", (i32)percent);
	buffer[len] = 0;
	b->len = snprintf(b->data, sizeof(b->data), b->fmt, buffer);

	return 1;
}

static BLOCK_INIT_FN(backlight_init)
{
	struct linux_backlight_data *lbd;
	b->user_data = lbd = push_struct(a, struct linux_backlight_data);

	Arena tmp = *a;
	char *path = alloc(&tmp, char, 4096);
	snprintf(path, 4096, "/sys/class/backlight/%s/max_brightness", (char *)b->arg);
	if (pscanf(path, "%ld", &lbd->max_brightness) != 1)
		die("backlight_init: failed to read max brightness\n");

	i64 len = snprintf(path, 4096, "/sys/class/backlight/%s/brightness", (char *)b->arg);
	lbd->brightness_path = path;
	a->beg += len;

	backlight_update(b, 0);
	add_file_watch(a, lbd->brightness_path, block_index, backlight_update);
}
