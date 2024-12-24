/* See LICENSE for license details. */
struct bat_arg {
	char *bat;      /* BAT name (ex. BAT0) */
	char *pre;      /* prefix for percentages less than thres */
	char *suf;      /* suffix for percentages less than thres */
	i32   thres;    /* % threshold to consider low (-1 to disable) */
	f32   interval; /* [s] */
};

struct linux_battery_data { i64 energy_full; f32 timer; };

static BLOCK_UPDATE_FN(battery_info_update)
{
	struct bat_arg *ba             = b->arg;
	struct linux_battery_data *lbd = b->user_data;

	if (!timer_update(&lbd->timer, ba->interval, dt))
		return 0;

	char *pre = ba->pre ? ba->pre : "";
	char *suf = ba->suf ? ba->suf : "";
	char path[4096], state[12];

	i32 h, m;
	i64 power_now, energy_now;
	f64 timeleft;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/energy_now", ba->bat);
	if (pscanf(path, "%ld", &energy_now) != 1)
		energy_now = 0;

	f32 percent = (100 * energy_now / (f64)lbd->energy_full) + 0.5;
	b32 warn    = percent < ba->thres;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", ba->bat);
	if (pscanf(path, "%12s", &state) != 1)
		snprintf(state, sizeof(state), "Unknown");

	/* NOTE(rnp): proper devices use negative power to indicate discharging but that
	 * is not always the case. The status string can mostly be trusted */
	if (!strcmp(state, "Discharging")) {
		snprintf(path, sizeof(path), "/sys/class/power_supply/%s/power_now", ba->bat);
		if (pscanf(path, "%ld", &power_now) != 1)
			power_now = 1;

		timeleft = energy_now / (f64)ABS(power_now);
		h = timeleft;
		m = (timeleft - (f64)h) * 60;

		i64 len = snprintf(buffer, sizeof(buffer), "%s%d%% (%d:%02d)%s", warn? pre : "",
		                   (i32)percent, h, m, warn? suf : "");
		buffer[len] = 0;
	} else {
		i64 len = snprintf(buffer, sizeof(buffer), "%s%d%% (%s)%s", warn? pre : "",
		                   (i32)percent, state, warn? suf : "");
		buffer[len] = 0;
	}
	b->len = snprintf(b->data, sizeof(b->data), b->fmt, buffer);

	return 1;
}

static BLOCK_INIT_FN(battery_info_init)
{
	struct bat_arg *ba = b->arg;
	struct linux_battery_data *lbd;
	b->user_data = lbd = push_struct(a, struct linux_battery_data);

	char path[4096];
	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/energy_full", ba->bat);
	if (pscanf(path, "%ld", &lbd->energy_full) != 1)
		die("battery_info_init: failed to read battery capacity\n");

	battery_info_update(b, 1);
}
