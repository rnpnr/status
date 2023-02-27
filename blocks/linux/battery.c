/* See LICENSE for license details. */
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "../../status.h"
#include "../../util.h"
#include "battery.h"

size_t
batinfo(struct Block *b)
{
	const struct bat_arg *ba = b->arg;
	char *pre = ba->pre ? ba->pre : "";
	char *suf = ba->suf ? ba->suf : "";
	char path[PATH_MAX], state[12];
	int perc, s;
	unsigned long power_now, energy_now, h, m;
	double timeleft;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity", ba->bat);
	if (pscanf(path, "%d", &perc) != 1)
		perc = 0;

	s = perc < ba->thres;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", ba->bat);
	if (pscanf(path, "%12s", &state) != 1)
		snprintf(state, sizeof(state), "Unknown");

	if (!strcmp(state, "Discharging")) {
		snprintf(path, sizeof(path),
		         "/sys/class/power_supply/%s/power_now", ba->bat);
		if (pscanf(path, "%lu", &power_now) != 1)
			power_now = 1;

		snprintf(path, sizeof(path),
		         "/sys/class/power_supply/%s/energy_now", ba->bat);
		if (pscanf(path, "%lu", &energy_now) != 1)
			energy_now = 0;

		timeleft = (double)energy_now / (double)power_now;
		h = timeleft;
		m = (timeleft - (double)h) * 60;

		snprintf(buf, sizeof(buf), "%s%d%% (%lu:%02lu)%s", s ? pre : "",
		         perc, h, m, s ? suf : "");
	} else {
		snprintf(buf, sizeof(buf), "%s%d%% (%s)%s", s ? pre : "", perc,
		         state, s ? suf : "");
	}

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
