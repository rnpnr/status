/* See LICENSE for license details. */
struct bat_arg {
	s8    bat;      /* BAT name (ex. s8("BAT0")) */
	char *pre;      /* prefix for percentages less than thres */
	char *suf;      /* suffix for percentages less than thres */
	i32   thres;    /* % threshold to consider low (-1 to disable) */
	f32   interval; /* [s] */
};

struct linux_battery_data { Stream path_base; i64 energy_full; f32 timer; };

static BLOCK_UPDATE_FN(battery_info_update)
{
	struct bat_arg *ba             = b->arg;
	struct linux_battery_data *lbd = b->user_data;

	if (!timer_update(&lbd->timer, ba->interval, dt))
		return 0;

	char *pre = ba->pre ? ba->pre : "";
	char *suf = ba->suf ? ba->suf : "";

	i32 h, m;
	i64 power_now, energy_now;
	f64 timeleft;

	size sidx = lbd->path_base.write_index;

	stream_push_s8(&lbd->path_base, s8("/energy_now"));
	energy_now = read_i64(stream_ensure_c_str(&lbd->path_base));
	lbd->path_base.write_index = sidx;

	f32 percent = (100 * energy_now / (f64)lbd->energy_full) + 0.5;
	b32 warn    = percent < ba->thres;

	char state_buffer[16] = {0};
	stream_push_s8(&lbd->path_base, s8("/status"));
	s8 state = s8_trim_space(read_s8(stream_ensure_c_str(&lbd->path_base),
	                                (s8){.len = sizeof(state_buffer),
	                                     .data = (u8 *)state_buffer}));
	if (state.len <= 0) state = s8("Unknown");
	lbd->path_base.write_index = sidx;

	/* NOTE(rnp): proper devices use negative power to indicate discharging but that
	 * is not always the case. The status string can mostly be trusted */
	if (s8_equal(state, s8("Discharging"))) {
		stream_push_s8(&lbd->path_base, s8("/power_now"));
		power_now = read_i64(stream_ensure_c_str(&lbd->path_base));
		if (!power_now) power_now = 1;
		lbd->path_base.write_index = sidx;

		timeleft = energy_now / (f64)ABS(power_now);
		h = timeleft;
		m = (timeleft - (f64)h) * 60;

		i64 len = snprintf(buffer, sizeof(buffer), "%s%d%% (%d:%02d)%s", warn? pre : "",
		                   (i32)percent, h, m, warn? suf : "");
		buffer[len] = 0;
	} else {
		i64 len = snprintf(buffer, sizeof(buffer), "%s%d%% (%s)%s", warn? pre : "",
		                   (i32)percent, (char *)state.data, warn? suf : "");
		buffer[len] = 0;
	}
	b->len = snprintf(b->data, sizeof(b->data), b->fmt, buffer);

	return 1;
}

#define LINUX_BAT_INFO_STRS \
	X("/energy_full")    \
	X("/energy_now")     \
	X("/power_now")      \
	X("/status")

static BLOCK_INIT_FN(battery_info_init)
{
	struct bat_arg *ba = b->arg;
	struct linux_battery_data *lbd;
	b->user_data = lbd = push_struct(a, struct linux_battery_data);

	size max_length = 0;
	#define X(cstr) if (sizeof(cstr) > max_length) max_length = sizeof(cstr);
	LINUX_BAT_INFO_STRS
	#undef X

	size needed_length = max_length + sizeof("/sys/class/power_supply/") - 1 + ba->bat.len;
	lbd->path_base     = stream_alloc(a, needed_length);

	stream_push_s8(&lbd->path_base, s8("/sys/class/power_supply/"));
	stream_push_s8(&lbd->path_base, ba->bat);
	size sidx = lbd->path_base.write_index;
	stream_push_s8(&lbd->path_base, s8("/energy_full"));
	lbd->energy_full = read_i64(stream_ensure_c_str(&lbd->path_base));
	if (!lbd->energy_full)
		die("battery_info_init: failed to read battery capacity\n");
	lbd->path_base.write_index = sidx;

	battery_info_update(b, 1);
}
