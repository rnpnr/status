/* See LICENSE for license details. */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mpd/client.h>
#include <X11/Xlib.h>

#include "status.h"
#include "util.h"
#include "config.h"

static int done = 0;
static int dflag = 0;
char buf[BUFLEN];

static Display *dpy;

static void
terminate(int signo)
{
	done = 1;
}

static void
setstatus(char *str)
{
	if (dflag) {
		puts(str);
		return;
	}

	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);	
}

static const char *
gettime(const char *fmt)
{
	time_t t = time(NULL);
	if (!(strftime(buf, sizeof(buf), fmt, localtime(&t))))
		return NULL;

	return buf;
}

static const char *
mpd(enum mpd_tag_type type)
{
	struct mpd_connection *conn = NULL;
	struct mpd_song *song = NULL;
	struct mpd_status *status = NULL;

	conn = mpd_connection_new(mpdhost, 0, 600);
	if (!conn || mpd_connection_get_error(conn))
		return NULL;

	mpd_command_list_begin(conn, true);
	mpd_send_status(conn);
	mpd_send_current_song(conn);
	mpd_command_list_end(conn);

	status = mpd_recv_status(conn);

	/* >= covers both PLAY and PAUSE */
	if (status && (mpd_status_get_state(status) >= MPD_STATE_PLAY)) {
		mpd_response_next(conn);
		song = mpd_recv_song(conn);
		snprintf(buf, sizeof(buf), "%s",
			mpd_song_get_tag(song, type, 0));
		mpd_song_free(song);
	} else
		snprintf(buf, sizeof(buf), "%s", "");

	if (status)
		mpd_status_free(status);
	mpd_response_finish(conn);
	mpd_connection_free(conn);

	return buf;
}

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	const char *s;
	char status[STATUSLEN];
	size_t len;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = terminate;
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	if (argc > 2)
		die("usage: %s [-d]\n", argv[0]);

	for (;argc && *argv; argc--) {
		switch ((argv++)[0][1]) {
		case 'd':
			dflag = 1;
		}
	}

	if (!dflag && !(dpy = XOpenDisplay(NULL)))
		die("XOpenDisplay: can't open display\n");

	for (len = 0; !done; sleep(1), len = 0) {
		s = mpd(MPD_TAG_ARTIST);
		len += snprintf(status + len, sizeof(status) - len, "[ %s -", s);

		s = mpd(MPD_TAG_TITLE);
		len += snprintf(status + len, sizeof(status) - len, " %s ]", s);

		s = getvol(alsacard, alsaoutput);
		len += snprintf(status + len, sizeof(status) - len, "[ %s ]", s);

		s = gettime(timefmt);
		len += snprintf(status + len, sizeof(status) - len, "[ %s ]", s);

		setstatus(status);
	}

	if (!dflag) {
		XStoreName(dpy, DefaultRootWindow(dpy), NULL);
		XCloseDisplay(dpy);
	}

	return 0;
}
