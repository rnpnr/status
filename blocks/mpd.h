/* to have access to tag types */
#include <mpd/tag.h>

struct mpd_arg {
	const char *host;
	const char *sep;
	const enum mpd_tag_type *tags;
	size_t ntags;
};

size_t mpd_tag(struct Block *b);
