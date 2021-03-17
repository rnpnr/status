#define BLOCKLEN 32
#define BLOCKPAD 4
struct Block {
	size_t (*const fn)(struct Block *b);
	const char *fmt;
	const int interval;
	const int signal;
	union { const char *s; const int i; } u;
	char curstr[BLOCKLEN];
	char prevstr[BLOCKLEN];
	size_t len;
};

extern char buf[BLOCKLEN - BLOCKPAD];
