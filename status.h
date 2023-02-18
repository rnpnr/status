#define BLOCKLEN 128
#define BLOCKPAD 4

struct Block {
	size_t (*const fn)(struct Block *b);
	const char *fmt;
	const int interval;
	const int signal;
	const void *arg;
	char curstr[BLOCKLEN];
	char prevstr[BLOCKLEN];
	size_t len;
};

extern char buf[BLOCKLEN - BLOCKPAD];
