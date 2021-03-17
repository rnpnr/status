/* See LICENSE for license details. */
#include <mpd/client.h>

#include "../status.h"
#include "../util.h"
#include "mpd.h"

size_t
mpd(struct Block *b)
{
	struct mpd_connection *conn = NULL;
	struct mpd_song *song = NULL;
	struct mpd_status *status = NULL;

	conn = mpd_connection_new(mpdhost, 0, 600);
	if (!conn || mpd_connection_get_error(conn))
		return bprintf(b->curstr, BLOCKLEN, b->fmt, "");

	mpd_command_list_begin(conn, true);
	mpd_send_status(conn);
	mpd_send_current_song(conn);
	mpd_command_list_end(conn);

	status = mpd_recv_status(conn);

	/* >= covers both PLAY and PAUSE */
	if (status && (mpd_status_get_state(status) >= MPD_STATE_PLAY)) {
		mpd_response_next(conn);
		song = mpd_recv_song(conn);
		bprintf(buf, sizeof(buf), "%s",
			mpd_song_get_tag(song, b->u.i, 0));
		mpd_song_free(song);
	} else
		bprintf(buf, sizeof(buf), "%s", "");

	if (status)
		mpd_status_free(status);
	mpd_response_finish(conn);
	mpd_connection_free(conn);

	return bprintf(b->curstr, BLOCKLEN, b->fmt, buf);
}
