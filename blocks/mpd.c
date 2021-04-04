/* See LICENSE for license details. */
#include <mpd/client.h>
#include <stdio.h>

#include "../status.h"
#include "../util.h"
#include "mpd.h"

static struct mpd_connection *conn;

static int
openconn(void)
{
	conn = mpd_connection_new(mpdhost, 0, 0);
	if (mpd_connection_get_error(conn)
	|| !mpd_connection_set_keepalive(conn, true)) {
		mpd_connection_free(conn);
		conn = NULL;
		return -1;
	}
	mpd_send_idle(conn);
	return 0;
}

size_t
mpd(struct Block *b)
{
	struct mpd_song *song = NULL;
	struct mpd_status *status = NULL;

	if (!conn && openconn() != 0)
		return snprintf(b->curstr, LEN(b->curstr), b->fmt, "");

	if (mpd_connection_get_error(conn)) {
		mpd_connection_free(conn);
		if (openconn() != 0)
			return snprintf(b->curstr, LEN(b->curstr), b->fmt, "");
	}

	mpd_run_noidle(conn);

	if ((status = mpd_run_status(conn))) {
		switch (mpd_status_get_state(status)) {
		case MPD_STATE_PAUSE:
		case MPD_STATE_PLAY:
			song = mpd_run_current_song(conn);
			snprintf(buf, sizeof(buf), "%s",
				mpd_song_get_tag(song, b->u.i, 0));
			mpd_song_free(song);
			break;
		case MPD_STATE_STOP:
		default:
			snprintf(buf, sizeof(buf), "%s", "");
		}
		mpd_status_free(status);
	} else
		snprintf(buf, sizeof(buf), "%s", "");

	mpd_send_idle(conn);

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, buf);
}
