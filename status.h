#define BLOCKLEN 32
struct Block {
	size_t (*const fn)(struct Block *b);
	const char *fmt;
	const int interval;
	const int signal;
	char curstr[BLOCKLEN];
	char prevstr[BLOCKLEN];
	size_t len;
};

extern char buf[BLOCKLEN];
