/* See LICENSE for license details. */
#include <mpd/client.h>
#include <stdio.h>

#include "../status.h"
#include "../util.h"
#include "mpd.h"

static struct mpd_connection *conn;

static int
open_conn(const char *host)
{
	conn = mpd_connection_new(host, 0, 0);
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
check_conn(const char *host)
{
	if (!mpd_connection_get_error(conn))
		return 0;

	mpd_connection_free(conn);
	return open_conn(host);
}

size_t
mpd_tag(struct Block *b)
{
	struct mpd_song *song = NULL;
	struct mpd_status *status = NULL;
	const struct mpd_arg *ma = b->arg;
	size_t i, len = 0;

	if ((!conn && open_conn(ma->host) != 0) || check_conn(ma->host) != 0)
		return 0;

	mpd_run_noidle(conn);

	if ((status = mpd_run_status(conn))) {
		switch (mpd_status_get_state(status)) {
		case MPD_STATE_PAUSE:
		case MPD_STATE_PLAY:
			song = mpd_run_current_song(conn);
			for (i = 0; i < ma->ntags; i++) {
				if (len != 0)
					len += snprintf(buf + len,
					                sizeof(buf) - len, "%s",
					                ma->sep ? ma->sep : "");
				len += snprintf(
				    buf + len, sizeof(buf) - len, "%s",
				    mpd_song_get_tag(song, ma->tags[i], 0));
			}
			mpd_song_free(song);
		case MPD_STATE_STOP:
		default:
			break;
		}
		mpd_status_free(status);
	}

	mpd_send_idle(conn);

	return len? snprintf(b->curstr, LEN(b->curstr), b->fmt, buf) : 0;
}
