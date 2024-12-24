/* See LICENSE for license details. */
struct linux_backlight_data {i64 max_brightness; char *brightness_path;};

static BLOCK_UPDATE_FN(backlight_update)
{
	if (dt > 0)
		return 0;

	struct linux_backlight_data *lbd = b->user_data;
	i64 current = read_i64(lbd->brightness_path);
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
	Stream path = stream_alloc(&tmp, KB(1));
	stream_push_s8(&path, s8("/sys/class/backlight/"));
	stream_push_s8(&path, *(s8 *)b->arg);
	size sidx = path.write_index;
	stream_push_s8(&path, s8("/max_brightness"));
	lbd->max_brightness = read_i64(stream_ensure_c_str(&path));
	if (!lbd->max_brightness)
		die("backlight_init: failed to read max brightness\n");
	path.write_index = sidx;

	stream_push_s8(&path, s8("/brightness"));
	path.buffer[path.write_index++] = 0;
	lbd->brightness_path = (char *)path.buffer;
	a->beg += path.write_index;

	backlight_update(b, 0);
	add_file_watch(a, lbd->brightness_path, block_index, backlight_update);
}
