struct bat_arg {
	char *bat; /* BAT name (ex. BAT0) */
	char *pre; /* prefix for percentages less than thres */
	char *suf; /* suffix for percentages less than thres */
	int thres; /* % threshold to consider low (-1 to disable) */
};

size_t batinfo(struct Block *b);
