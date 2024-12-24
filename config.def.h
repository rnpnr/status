#include "blocks/date.c"
#include "blocks/linux/battery_info.c"
//#include "blocks/linux/backlight.c"
//#include "blocks/linux/volume.c"
//#include "blocks/mpd.c"
//#include "blocks/script.c"

/* mpd_arg.host can be NULL to use the MPD_HOST env variable */
// static enum mpd_tag_type tags[] = { MPD_TAG_TITLE, MPD_TAG_ARTIST };
// static struct mpd_arg ma = { "localhost", "|", tags, 2 };

/* alsa card and sink */
/* card is found with 'aplay -L', default is probably correct */
// static struct vol_arg va = { "default", "Speaker" };

/* check blocks/xxx/battery_info.c for info */
static struct bat_arg ba  = {.bat = s8("BAT0"), .interval = 30};

/* check blocks/date.c for info */
static struct date_arg da = {.fmt = "%R", .interval = 30};

/* status block definitions
 *
 * block            description                    arg (ex)
 *
 * battery_info     battery percentage and status  (struct bat_arg *)
 * backlight        percentage                     (char *) backlight name (intel_backlight)
 * date             date and time                  (struct date_arg *)
 */

/* NOTE: X(name, statusline_format, argument) */
#define BLOCKS \
	X(battery_info, "[ %s ]", &ba) \
	X(date,         "[ %s ]", &da)
