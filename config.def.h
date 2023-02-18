#include "blocks/date.h"
#include "blocks/linux/battery.h"
#include "blocks/linux/blight.h"
#include "blocks/linux/volume.h"
#include "blocks/mpd.h"
#include "blocks/script.h"

/* update intervals: SEC+NANO gives sleep interval */
/* SEC must be >= 0 and 0 <= NANO <= 999999999 */
#define INTERVAL_SEC  1
#define INTERVAL_NANO 0

/* mpd_arg.host can be NULL to use the MPD_HOST env variable */
const enum mpd_tag_type tags[] = { MPD_TAG_TITLE, MPD_TAG_ARTIST };
const struct mpd_arg ma = { "localhost", "|", tags, 2 };

/* alsa card and sink */
/* card is found with 'aplay -L', default is probably correct */
const struct vol_arg va = { "default", "Speaker" };

/* status block definitions
 *
 * function  description                    arg (ex)
 *
 * batinfo   battery percentage and status  (char *) battery name (BAT0)
 *                                          0 on OpenBSD
 * blight    backlight percentage           (char *) backlight name (intel_backlight)
 * date      date and time                  (char *) time fmt string (%R)
 * volume    ALSA volume percentage         (struct vol_arg *)
 * mpd_tag   reads tag from current song    (struct mpd_arg *)
 * script    run specified script           (char *) full script (echo foo | bar)
 *
 *
 * interval * INTERVAL above gives actual update interval, 0 only updates
 * at the start and when signaled, -1 only updates when signaled
 */
struct Block blks[] = {
/*	  fn         fmt        interval  signal  arg */
	{ batinfo,   "[ %s ]",  30,       0,      "BAT0" },
	{ date,      "[ %s ]",  20,       0,      "%R" },
	{ NULL },
};
