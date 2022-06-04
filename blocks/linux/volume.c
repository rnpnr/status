/* See LICENSE for license details. */
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../status.h"
#include "../../util.h"
#include "volume.h"

size_t
getvol(struct Block *b)
{
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	snd_mixer_elem_t *elem;
	const char *str = "muted";

	int notmuted;
	long vol = 0, min, max;

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, alsacard);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_malloc(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, b->u.s);
	elem = snd_mixer_find_selem(handle, sid);

	snd_mixer_selem_get_playback_switch(elem, 0, &notmuted);

	if (snd_mixer_selem_has_playback_volume(elem)) {
		snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &vol);
		snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

		/* covert from raw value to percent */
		vol = (double)(vol - min) / (double)(max - min) * 100;
	}

	/* don't change to snd_mixer_elem_free() it leaks memory */
	snd_mixer_close(handle);
	snd_mixer_selem_id_free(sid);

	if (notmuted) {
		if (vol) {
			snprintf(buf, sizeof(buf), "%ld%%", vol);
			str = buf;
		} else
			str = "on";
	}

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, str);
}
