#include "blocks/battery.h"
#include "blocks/gettime.h"
#include "blocks/mpd.h"
#include "blocks/volume.h"

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

/* status block definitions */
/* interval * INTERVAL above gives actual update interval, 0 only updates
 * at the start and when signaled, -1 only updates when signaled  */
struct Block blks[] = {
/*        fn         fmt                    interval   signal  arg */
	{ mpd_tag,   "[ %s ",               0,         1,      { .i = MPD_TAG_ARTIST } },
	{ mpd_tag,   "- %s ]",              0,         1,      { .i = MPD_TAG_TITLE } },
	{ batinfo,   "[ %s ]",              30,        0,      { .s = "BAT0" } },
	{ getvol,    "[ %s ]",              0,         2,      { .s = "Speaker" } },
	{ gettime,   "[ %R ]",              20,        0,      {0} },
	{ NULL },
};
