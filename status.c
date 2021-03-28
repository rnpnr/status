/* See LICENSE for license details. */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "status.h"
#include "util.h"
#include "config.h"

#define STATUSLEN ((LEN(blks) - 1) * BLOCKLEN + 1)

static int done = 0;
static int dflag = 0;
char buf[BLOCKLEN - BLOCKPAD];

static Display *dpy;
static struct Block *dirty;

static void
terminate(int signo)
{
	done = 1;
}

static void
updateblock(struct Block *b)
{
	b->len = b->fn(b);
	if (memcmp(b->curstr, b->prevstr, b->len)) {
		memcpy(b->prevstr, b->curstr, b->len);
		if (!dirty || b < dirty)
			dirty = b;
	}
}

static void
updatestatus(void)
{
	static char status[STATUSLEN];
	struct Block *b;
	char *s = status;

	for (b = blks; b < dirty; b++)
		s += b->len;

	for (; b->fn; b++) {
		memcpy(s, b->curstr, b->len);
		s += b->len;
	}
	s[0] = '\0';
	dirty = NULL;

	if (dflag) {
		puts(status);
		return;
	}

	XStoreName(dpy, DefaultRootWindow(dpy), status);
	XSync(dpy, False);	
}

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	struct Block *b;

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

	for (; !done; sleep(1))
		for (b = blks; b->fn; b++) {
			updateblock(b);
			if (dirty)
				updatestatus();
		}

	if (!dflag) {
		XStoreName(dpy, DefaultRootWindow(dpy), NULL);
		XCloseDisplay(dpy);
	}

	return 0;
}
