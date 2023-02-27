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
	const struct bat_arg ba = b->arg;
	char *pre = ba->pre ? ba->pre : "";
	char *suf = ba->suf ? ba->suf : "";
	struct apm_power_info pi;
	int fd, s;

	if ((fd = open("/dev/apm", O_RDONLY)) < 0)
		die("open\n");

	if ((ioctl(fd, APM_IOC_GETPOWER, &pi)) < 0) {
		close(fd);
		die("ioctl\n");
	}
	close(fd);

	s = pi.battery_life < ba->thres;

	switch (pi.ac_state) {
	case APM_AC_OFF:
		snprintf(buf, sizeof(buf), "%s%d%% (%d:%02d)%s", s ? pre : "",
		         pi.battery_life, pi.minutes_left / 60,
		         pi.minutes_left % 60, s ? suf : "");
	case APM_AC_ON:
	case APM_BATT_CHARGING:
		snprintf(buf, sizeof(buf), "%d%% (ac)", pi.battery_life);
	default:
		snprintf(buf, sizeof(buf), "%s%d%% (unknown)%s", s ? pre : "",
		         pi.battery_life, s ? suf : "");
	}
	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
