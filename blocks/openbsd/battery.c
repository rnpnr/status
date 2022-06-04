/* See LICENSE for license details. */
#include <fcntl.h>
#include <machine/apmvar.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../../status.h"
#include "../../util.h"
#include "battery.h"

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
		snprintf(buf, sizeof(buf), "%d%% (%d:%02d)", pi.battery_life,
			pi.minutes_left / 60, pi.minutes_left % 60);
	case APM_AC_ON:
	case APM_BATT_CHARGING:
		snprintf(buf, sizeof(buf), "%d%% (ac)", pi.battery_life);
	default:
		snprintf(buf, sizeof(buf), "%d%% (unknown)", pi.battery_life);
	}
	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
