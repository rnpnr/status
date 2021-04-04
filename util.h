#define LEN(a) (sizeof(a) / sizeof(*a))

void die(const char *errstr, ...);
int pscanf(const char *path, const char *fmt, ...);
