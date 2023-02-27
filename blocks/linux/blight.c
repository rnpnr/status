/* See LICENSE for license details. */
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "../../status.h"
#include "../../util.h"
#include "blight.h"

size_t
blight(struct Block *b)
{
	char path[PATH_MAX];
	int perc;
	unsigned long max, now;

	snprintf(path, sizeof(path), "/sys/class/backlight/%s/brightness", (char *)b->arg);
	if (pscanf(path, "%lu", &now) != 1)
		now = 0;

	snprintf(path, sizeof(path), "/sys/class/backlight/%s/max_brightness", (char *)b->arg);
	if (pscanf(path, "%lu", &max) != 1)
		/* avoid divison by 0 */
		max = 1;

	perc = 100 * now / max;
	snprintf(buf, sizeof(buf), "%d%%", perc);

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
