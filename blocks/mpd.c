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
	if (mpd_connection_get_error(conn)) {
		mpd_connection_free(conn);
		conn = NULL;
		return -1;
	}
	return 0;
}

size_t
mpd(struct Block *b)
{
	struct mpd_song *song = NULL;
	struct mpd_status *status = NULL;

	if (!conn && openconn() != 0)
		return snprintf(b->curstr, BLOCKLEN, b->fmt, "");

	if (mpd_connection_get_error(conn)) {
		mpd_connection_free(conn);
		if (openconn() != 0)
			return snprintf(b->curstr, BLOCKLEN, b->fmt, "");
	}

	if (mpd_send_status(conn))
		status = mpd_recv_status(conn);

	/* >= covers both PLAY and PAUSE */
	if (status && (mpd_status_get_state(status) >= MPD_STATE_PLAY)) {
		mpd_send_current_song(conn);
		song = mpd_recv_song(conn);
		snprintf(buf, sizeof(buf), "%s",
			mpd_song_get_tag(song, b->u.i, 0));
		mpd_song_free(song);
	} else
		snprintf(buf, sizeof(buf), "%s", "");

	if (status)
		mpd_status_free(status);
	mpd_response_finish(conn);

	return snprintf(b->curstr, BLOCKLEN, b->fmt, buf);
}
