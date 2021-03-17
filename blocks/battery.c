/* See LICENSE for license details. */
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "../status.h"
#include "../util.h"
#include "battery.h"

#if defined(__linux__)
size_t
batinfo(struct Block *b)
{
	static char path[PATH_MAX], state[12];
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

		bprintf(buf, sizeof(buf), "%d%% (%d:%02d)", perc, h, m);
	} else
		bprintf(buf, sizeof(buf), "%d%% (%s)", perc, state);

	return bprintf(b->curstr, BLOCKLEN, b->fmt, buf);
}

#elif defined(__OpenBSD__)
#include <fcntl.h>
#include <machine/apmvar.h>
#include <sys/ioctl.h>
#include <unistd.h>

size_t
batinfo(struct Block *b)
{
	struct apm_power_info pi;
	int fd;

	if ((fd = open("/dev/apm", O_RDONLY)) < 0)
		die("open\n");

	if ((ioctl(fd, APM_IOC_GETPOWER, &pi)) < 0) {
		close(fd);
		die("ioctl\n");
	}
	close(fd);

	switch (pi.ac_state) {
	case APM_AC_OFF:
		bprintf(buf, sizeof(buf), "%d%% (%d:%02d)", pi.battery_life,
			pi.minutes_left / 60, pi.minutes_left % 60);
	case APM_AC_ON:
	case APM_BATT_CHARGING:
		bprintf(buf, sizeof(buf), "%d%% (ac)", pi.battery_life);
	default:
		bprintf(buf, sizeof(buf), "%d%% (unknown)", pi.battery_life);
	}
	return bprintf(b->curstr, BLOCKLEN, b->fmt, buf);
}
#endif
