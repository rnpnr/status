#include "status.h"

#if defined(__linux__)
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

int
getvol(const char *card, const char *output)
{
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	snd_mixer_elem_t *elem;

	long vol, min, max;

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, card);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_malloc(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, output);
	elem = snd_mixer_find_selem(handle, sid);

	snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &vol);
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

	/* covert from raw value to percent */
	vol = (double)vol / (double)(max - min) * 100;

	snd_mixer_close(handle);
	snd_mixer_selem_id_free(sid);
	return (int)vol;
}

#endif
