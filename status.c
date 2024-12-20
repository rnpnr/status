/* See LICENSE for license details. */
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define LEN(a) (sizeof(a) / sizeof(*a))
#define BLOCKLEN 128
#define BLOCKPAD 4
#define STATUSLEN ((LEN(blks) - 1) * BLOCKLEN + 1)

struct Block {
	size_t (*fn)(struct Block *b);
	const char *fmt;
	int interval;
	int signal;
	void *arg;
	char curstr[BLOCKLEN];
	char prevstr[BLOCKLEN];
	size_t len;
};

static char buf[BLOCKLEN - BLOCKPAD];
static Display *dpy;
static int dflag = 0;
static sigset_t blocksigmask;
static struct Block *dirty;

static void
die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(1);
}

static int
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

#include "config.h"

static void
terminate(int signo)
{
	if (!dflag) {
		XStoreName(dpy, DefaultRootWindow(dpy), NULL);
		XCloseDisplay(dpy);
	}

	exit(0);
}

static void
updateblock(struct Block *b)
{
	b->len = b->fn(b);
	if ((b->len == 0 && b->prevstr[0] != 0) ||
	    memcmp(b->curstr, b->prevstr, b->len)) {
		if (b->len == 0)
			b->prevstr[0] = b->curstr[0] = 0;
		else
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

	/* add signals to blocksigmask */
	sigemptyset(&blocksigmask);
	for (b = blks; b->fn; b++) {
		if (b->signal <= 0)
			continue;

		if (b->signal > SIGRTMAX - SIGRTMIN)
			die("SIGRTMIN + %d exceeds SIGRTMAX\n", b->signal);

		sigaddset(&blocksigmask, SIGRTMIN + b->signal);
	}

	/* handle terminating signals */
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
	for (b = blks; b->fn; b++)
		if (b->signal > 0)
			sigaction(SIGRTMIN + b->signal, &sa, NULL);

	/* start with signals blocked */
	sigprocmask(SIG_BLOCK, &blocksigmask, NULL);
}

static void
statusinit(void)
{
	struct Block *b;

	setupsigs();

	if (!dflag && !(dpy = XOpenDisplay(NULL)))
		die("XOpenDisplay: can't open display\n");

	/* initialize blocks before first print */
	for (b = blks; b->fn; b++)
		if (b->interval != -1)
			updateblock(b);
	updatestatus();
}

static void
statusloop(void)
{
	unsigned int i = 0;
	struct Block *b;
	struct timespec t;

	for (;;) {
		sigprocmask(SIG_UNBLOCK, &blocksigmask, NULL);
		t.tv_sec = INTERVAL_SEC;
		t.tv_nsec = INTERVAL_NANO;
		while (nanosleep(&t, &t) == -1);
		sigprocmask(SIG_BLOCK, &blocksigmask, NULL);

		for (b = blks; b->fn; b++)
			if (b->interval > 0 && i % b->interval == 0)
				updateblock(b);
		if (dirty)
			updatestatus();
		i++;
	}
}

int
main(int argc, char *argv[])
{
	int i;
	char *argv0 = *argv;

	for (argv++; --argc && *argv && argv[0][0] == '-' && argv[0][1]; argv++) {
		if (argv[0][1] == '-' && argv[0][2] == '\0') {
			argv++;
			argc--;
			break;
		}

		for (i = 1; argv[0][i]; i++)
			switch (argv[0][i]) {
			case 'd':
				dflag = 1;
				break;
			default:
				die("usage: %s [-d]\n", argv0);
			}
	}

	statusinit();
	statusloop();

	return 0;
}
