#include <fcntl.h>
#include <machine/apmvar.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "status.h"
#include "util.h"

const char *
getvol(const char *card, const char *output)
{
	return NULL;
}

const char *
batinfo(const char *bat)
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
		return smprintf("%d%% (%d:%02d)", pi.battery_life,
				pi.minutes_left / 60, pi.minutes_left % 60);
	case APM_AC_ON:
	case APM_BATT_CHARGING:
		return bprintf("%d%% (ac)", pi.battery_life);
	default:
		return bprintf("%d%% (unknown)", pi.battery_life);
	}
}
