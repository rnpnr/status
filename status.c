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
static sigset_t blocksigmask;

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

static void
sighandler(int signo, siginfo_t *info, void *context)
{
	struct Block *b;

	signo -= SIGRTMIN;
	for (b = blks; b->fn; b++)
		if (b->signal == signo)
			updateblock(b);
	if (dirty)
		updatestatus();
}

static void
setupsigs(void)
{
	int i;
	struct Block *b;
	struct sigaction sa;

	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = terminate;
	sigaction(SIGHUP,  &sa, NULL);
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	/* ignore unused realtime signals */
	sa.sa_handler = SIG_IGN;
	for (i = SIGRTMIN + 1; i <= SIGRTMAX; i++)
		sigaction(i, &sa, NULL);

	/* handle update signals for blocks */
	sa.sa_flags = SA_NODEFER | SA_RESTART | SA_SIGINFO;
	sa.sa_mask = blocksigmask;
	sa.sa_sigaction = sighandler;
	for (b = blks; b->fn; b++) {
		if (SIGRTMIN + b->signal > SIGRTMAX)
			die("SIGRTMIN + %d exceeds SIGRTMAX\n", b->signal);
		else if (b->signal > 0)
			sigaction(SIGRTMIN + b->signal, &sa, NULL);
	}
}

int
main(int argc, char *argv[])
{
	struct Block *b;

	if (argc > 2)
		die("usage: %s [-d]\n", argv[0]);

	for (;argc && *argv; argc--) {
		switch ((argv++)[0][1]) {
		case 'd':
			dflag = 1;
		}
	}

	setupsigs();

	if (!dflag && !(dpy = XOpenDisplay(NULL)))
		die("XOpenDisplay: can't open display\n");

	/* initialize blocks before first print */
	for (b = blks; b->fn; b++)
		if (b->interval != -1)
			updateblock(b);
	updatestatus();

	for (; !done; sleep(1))
		for (b = blks; b->fn; b++) {
			if (b->interval > 0)
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
