#include "blocks/gettime.h"
#include "blocks/linux/battery.h"
#include "blocks/linux/blight.h"
#include "blocks/linux/volume.h"
#include "blocks/mpd.h"
#include "blocks/script.h"

/* update intervals: SEC+NANO gives sleep interval */
/* SEC must be >= 0 and 0 <= NANO <= 999999999 */
#define INTERVAL_SEC  1
#define INTERVAL_NANO 0

/* host for connecting to MPD, set to NULL for the MPD_HOST env variable */
const char *mpdhost = "localhost";

/* alsa card and output */
/* card is found with 'aplay -L', default is probably correct
 * output is specified as an arg */
const char *alsacard = "default";

/* status block definitions
 *
 * function  description                    arg (ex)
 *
 * batinfo   battery percentage and status  (.s) battery name (BAT0)
 *                                          0 on OpenBSD
 * blight    backlight percentage           (.s) backlight name (intel_backlight)
 * date      date and time                  (.s) time fmt string (%R)
 * getvol    ALSA volume percentage         (.s) sink name (Speaker)
 * mpd_tag   reads tag from current song    (.i) enum mpd_tag_type (MPD_TAG_TITLE)
 * script    run specified script           (.s) full script (echo foo | bar)
 *
 *
 * interval * INTERVAL above gives actual update interval, 0 only updates
 * at the start and when signaled, -1 only updates when signaled
 */
struct Block blks[] = {
/*	  fn         fmt        interval  signal  arg */
	{ batinfo,   "[ %s ]",  30,       0,      { .s = "BAT0" } },
	{ date,      "[ %s ]",  20,       0,      { .s = "%R" } },
	{ NULL },
};
