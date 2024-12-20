/* See LICENSE for license details. */
#include <limits.h>

struct bat_arg {
	char *bat; /* BAT name (ex. BAT0) */
	char *pre; /* prefix for percentages less than thres */
	char *suf; /* suffix for percentages less than thres */
	int thres; /* % threshold to consider low (-1 to disable) */
};

static size_t
batinfo(struct Block *b)
{
	struct bat_arg *ba = b->arg;
	char *pre = ba->pre ? ba->pre : "";
	char *suf = ba->suf ? ba->suf : "";
	char path[PATH_MAX], state[12];
	int s, perc, h, m;
	long power_now, energy_now, energy_full;
	double timeleft;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/energy_full", ba->bat);
	if (pscanf(path, "%ld", &energy_full) != 1)
		energy_full = 1;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/energy_now", ba->bat);
	if (pscanf(path, "%ld", &energy_now) != 1)
		energy_now = 0;

	perc = (100 * energy_now / (double)energy_full);
	s = perc < ba->thres;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", ba->bat);
	if (pscanf(path, "%12s", &state) != 1)
		snprintf(state, sizeof(state), "Unknown");

	/* NOTE(rnp): proper devices use negative power to indicate discharging but that
	 * is not always the case. The status string can mostly be trusted */
	if (!strcmp(state, "Discharging")) {
		snprintf(path, sizeof(path), "/sys/class/power_supply/%s/power_now", ba->bat);
		if (pscanf(path, "%ld", &power_now) != 1)
			power_now = 1;

		timeleft = (double)energy_now / (double)ABS(power_now);
		h = timeleft;
		m = (timeleft - (double)h) * 60;

		snprintf(buf, sizeof(buf), "%s%d%% (%d:%02d)%s", s ? pre : "",
		         perc, h, m, s ? suf : "");
	} else {
		snprintf(buf, sizeof(buf), "%s%d%% (%s)%s", s ? pre : "", perc,
		         state, s ? suf : "");
	}

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
