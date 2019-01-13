void die(const char *errstr, ...);
char *smprintf(const char *fmt, ...);
int pscanf(const char *path, const char *fmt, ...);

int getvol(const char *card, const char *output);
char *batinfo(const char *bat);
