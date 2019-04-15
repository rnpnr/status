#include <limits.h>
#include <stdio.h>

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

#include "status.h"

const char *
getvol(const char *card, const char *output)
{
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	snd_mixer_elem_t *elem;

	int notmuted;
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
	snd_mixer_selem_get_playback_switch(elem, 0, &notmuted);

	/* covert from raw value to percent */
	vol = (double)(vol - min) / (double)(max - min) * 100;

	snd_mixer_close(handle);
	snd_mixer_selem_id_free(sid);

	if (notmuted)
		return bprintf("%d%%", (int)vol);
	return bprintf("(muted)");
}

const char *
batinfo(const char *bat)
{
	int perc;
	unsigned long power_now, energy_now, h, m;
	double timeleft;
	char path[PATH_MAX], state[12];

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/capacity", bat);
	if (pscanf(path, "%d", &perc) != 1)
		perc = 0;

	snprintf(path, sizeof(path), "/sys/class/power_supply/%s/status", bat);
	if (pscanf(path, "%12s", &state) != 1)
		snprintf(state, sizeof(state), "Unknown");

	if (!strcmp(state, "Discharging")) {
		snprintf(path, sizeof(path),
			"/sys/class/power_supply/%s/power_now", bat);
		if (pscanf(path, "%lu", &power_now) != 1)
			power_now = 1;

		snprintf(path, sizeof(path),
			"/sys/class/power_supply/%s/energy_now", bat);
		if (pscanf(path, "%lu", &energy_now) != 1)
			energy_now = 0;

		timeleft = (double)energy_now / (double)power_now;
		h = timeleft;
		m = (timeleft - (double)h) * 60;

		return bprintf("%d%% (%d:%02d)", perc, h, m);
	}

	return bprintf("%d%% (%s)", perc, state);
}
