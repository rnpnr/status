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

static int done = 0;
static int dflag = 0;
char buf[BLOCKLEN];

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

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	char status[STATUSLEN];
	size_t len;
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

	for (len = 0; !done; sleep(1), len = 0) {
		for (b = blks; b->fn; b++) {
			b->len = b->fn(b);
			len += snprintf(status + len, sizeof(status) - len, "%s", b->curstr);
		}
		setstatus(status);
	}

	if (!dflag) {
		XStoreName(dpy, DefaultRootWindow(dpy), NULL);
		XCloseDisplay(dpy);
	}

	return 0;
}
