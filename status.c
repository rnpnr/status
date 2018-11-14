#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mpd/client.h>
#include <X11/Xlib.h>

#include "status.h"
#include "config.h"

static int done = 0;
static char buf[1024];

static Display *dpy;

static void
die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(1);
}

static void
terminate(const int signo)
{
	(void)signo;

	done = 1;
}

static char *
smprintf(const char *fmt, ...)
{
	va_list ap;
	char *ret;
	int len;

	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	if (!(ret = malloc(++len)))
		die("malloc\n");

	va_start(ap, fmt);
	vsnprintf(ret, len, fmt, ap);
	va_end(ap);

	return ret;
}

static void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);	
}

static char *
gettime(const char *fmt)
{
	const time_t t = time(NULL);
	if (!(strftime(buf, sizeof(buf), fmt, localtime(&t))))
		return smprintf("");

	return smprintf("%s", buf);
}

static char *
mpd(enum mpd_tag_type type)
{
	struct mpd_connection *conn = NULL;
	struct mpd_song *song = NULL;
	struct mpd_status *status = NULL;

	conn = mpd_connection_new(mpdhost, 0, 600);
	if (!conn || mpd_connection_get_error(conn))
		return smprintf("");

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

	mpd_response_finish(conn);
	mpd_connection_free(conn);

	return smprintf("%s", buf);
}

int
main(void)
{
	struct sigaction sa;
	char *status;
	char *time;
	char *artist, *song;
	int vol;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = terminate;
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	if (!(dpy = XOpenDisplay(NULL)))
		die("XOpenDisplay: can't open display\n");

	for (; !done; sleep(1)) {
		time = gettime(timefmt);
		song = mpd(MPD_TAG_TITLE);
		artist = mpd(MPD_TAG_ARTIST);
		vol = getvol(alsacard, alsaoutput);

		status = smprintf("[ %s - %s ][ %d%% ][ %s ]", artist, song,
				vol, time);
		setstatus(status);

		free(time);
		free(song);
		free(artist);
		free(status);
	}

	XStoreName(dpy, DefaultRootWindow(dpy), NULL);
	XCloseDisplay(dpy);

	return 0;
}
