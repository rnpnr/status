#define LEN(a) sizeof(a) / sizeof(*a)

void die(const char *errstr, ...);
size_t bprintf(char *buf, size_t buflen, const char *fmt, ...);
int pscanf(const char *path, const char *fmt, ...);
