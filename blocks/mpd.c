/* See LICENSE for license details. */
#include <mpd/client.h>
#include <stdio.h>

#include "../status.h"
#include "../util.h"
#include "mpd.h"

static struct mpd_connection *conn;

static int
open_conn(void)
{
	conn = mpd_connection_new(mpdhost, 0, 0);
	if (!mpd_connection_get_error(conn)
	&& mpd_connection_set_keepalive(conn, true)) {
		mpd_send_idle(conn);
		return 0;
	}
	mpd_connection_free(conn);
	conn = NULL;
	return -1;
}

static int
check_conn(void)
{
	if (!mpd_connection_get_error(conn))
		return 0;

	mpd_connection_free(conn);
	return open_conn();
}

size_t
mpd_tag(struct Block *b)
{
	struct mpd_song *song = NULL;
	struct mpd_status *status = NULL;
	const char *str = "";

	if ((!conn && open_conn() != 0) || check_conn() != 0)
		return snprintf(b->curstr, LEN(b->curstr), b->fmt, str);

	mpd_run_noidle(conn);

	if ((status = mpd_run_status(conn))) {
		switch (mpd_status_get_state(status)) {
		case MPD_STATE_PAUSE:
		case MPD_STATE_PLAY:
			song = mpd_run_current_song(conn);
			str = mpd_song_get_tag(song, b->u.i, 0);
			mpd_song_free(song);
			break;
		case MPD_STATE_STOP:
		default:
		}
		mpd_status_free(status);
	}

	mpd_send_idle(conn);

	return snprintf(b->curstr, LEN(b->curstr), b->fmt, str);
}
