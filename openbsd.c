#include "status.h"

#if defined(__OpenBSD__)

#include <fcntl.h>
#include <machine/apmvar.h>
#include <sys/ioctl.h>
#include <unistd.h>

int
getvol(const char *card, const char *output)
{
	return 0;
}

char *
batinfo(void)
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
	case APM_BATT_CHARGING:
		return smprintf("%d%% (charging)", pi.battery_life);
	case APM_AC_ON:
		return smprintf("%d%% (external)", pi.battery_life);
	default:
		return smprintf("%d%% (unknown)", pi.battery_life);
	}
}

#endif
