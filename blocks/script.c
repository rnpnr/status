/* See LICENSE for license details. */
static BLOCK_UPDATE_FN(script_update)
{
	char *out = "";

	/* TODO(rnp): don't use c-runtime for this */
	FILE *fp  = popen(b->arg, "r");
	if (fp) {
		if (fgets(buffer, sizeof(buffer), fp)) {
			buffer[strcspn(buffer, "\n")] = 0;
			out = buffer;
		}
		pclose(fp);
	}
	b->len = snprintf(b->data, sizeof(b->data), b->fmt, out);
}

static BLOCK_INIT_FN(script_init)
{
	script_update(b);
}
