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
static struct bat_arg ba  = {.bat = s8("BAT0")};

/* backlight name (/sys/class/backlight/xxx) */
//static s8 linux_backlight = s8("xxx");

/* status block definitions
 *
 * block            description                    arg (ex)
 *
 * battery_info     battery percentage and status  (struct bat_arg *)
 * backlight        percentage                     (s8 *) backlight name
 * date             date and time                  (char *) fmt ("%R")
 */

/* NOTE: X(name, statusline_format, interval, argument) *
 * interval == 0 means never update from timer          */
#define BLOCKS \
	X(battery_info, "[ %s ]", 30, &ba) \
	X(date,         "[ %s ]", 30, "%R")
