#include "blocks/battery.h"
#include "blocks/gettime.h"
#include "blocks/mpd.h"
#include "blocks/volume.h"

/* host for connecting to MPD, set to NULL for the MPD_HOST env variable */
const char *mpdhost = "localhost";

/* alsa card and output */
/* card is found with 'aplay -L', default is probably correct
 * output is specified as an arg */
const char *alsacard = "default";

/* status block definitions */
struct Block blks[] = {
/*        fn         fmt                    interval   signal  arg */
	{ mpd,       "[ %s ",               0,         0,      { .i = MPD_TAG_ARTIST } },
	{ mpd,       "- %s ]",              0,         0,      { .i = MPD_TAG_TITLE } },
	{ batinfo,   "[ %s ]",              0,         0,      { .s = "BAT0" } },
	{ getvol,    "[ %s ]",              0,         0,      { .s = "Speaker" } },
	{ gettime,   "[ %R ]",              20,        0,      {0} },
	{ NULL },
};
