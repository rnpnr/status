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
	char path[PATH_MAX], state[12], *bat = (char *)b->arg;
	int perc;
	unsigned long power_now, energy_now, h, m;
	double timeleft;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity", bat);
	if (pscanf(path, "%d", &perc) != 1)
		perc = 0;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", bat);
	if (pscanf(path, "%12s", &state) != 1)
		snprintf(state, sizeof(state), "Unknown");

	if (!strcmp(state, "Discharging")) {
		snprintf(path, sizeof(path),
		         "/sys/class/power_supply/%s/power_now", bat);
		if (pscanf(path, "%lu", &power_now) != 1)
			power_now = 1;

		snprintf(path, sizeof(path),
		         "/sys/class/power_supply/%s/energy_now", bat);
		if (pscanf(path, "%lu", &energy_now) != 1)
			energy_now = 0;

		timeleft = (double)energy_now / (double)power_now;
		h = timeleft;
		m = (timeleft - (double)h) * 60;

		snprintf(buf, sizeof(buf), "%d%% (%lu:%02lu)", perc, h, m);
	} else
		snprintf(buf, sizeof(buf), "%d%% (%s)", perc, state);

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
