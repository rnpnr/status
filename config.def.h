#include "blocks/gettime.h"
#include "blocks/volume.h"

#define STATUSLEN 1024

/* host for connecting to MPD, set to NULL for the MPD_HOST env variable */
static const char *mpdhost = "localhost";

/* alsa card and output */
/* card is whatever alsamixer lists as card, default is probably correct
   output is the output from that specific card you want the vol from */
static const char *alsacard = "default";
static const char *alsaoutput = "Speaker";

/* main battery in system */
/* found in /sys/class/power_supply/ */
static const char *bat = "BAT0";

/* status block definitions */
struct Block blks[] = {
/*        fn         fmt                    interval   signal */
	{ getvol,    "[ %s ]",              0,         0 },
	{ gettime,   "[ %R ]",              20,        0 },
	{ NULL },
};
