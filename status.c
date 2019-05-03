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

void
die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(1);
}

static void
terminate(int signo)
{
	done = 1;
}

const char *
bprintf(const char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	return (ret < 0)? NULL : buf;
}

int
pscanf(const char *path, const char *fmt, ...)
{
	FILE *fp;
	va_list ap;
	int ret;

	if (!(fp = fopen(path, "r")))
		return -1;

	va_start(ap, fmt);
	ret = vfscanf(fp, fmt, ap);
	va_end(ap);
	fclose(fp);

	return (ret == EOF) ? -1 : ret;
}

static void
setstatus(char *str)
{
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
main(void)
{
	struct sigaction sa;
	const char *s;
	char status[STATUSLEN];
	size_t len;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = terminate;
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	if (!(dpy = XOpenDisplay(NULL)))
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

	XStoreName(dpy, DefaultRootWindow(dpy), NULL);
	XCloseDisplay(dpy);

	return 0;
}
