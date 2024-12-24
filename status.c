/* See LICENSE for license details. */
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define ABS(a)         ((a) < 0 ? -(a) : (a))
#define ARRAY_COUNT(a) (sizeof(a) / sizeof(*a))
#define BLOCKLEN 128

#define TICK_RATE_SECONDS     1
#define TICK_RATE_NANOSECONDS 0

#define SLLPush(sll, new) do {     \
	(new)->next = (sll)->next; \
	(sll)->next = (new);       \
} while(0)

#define KB(n) ((n) << 10ULL)

#ifndef asm
#define asm __asm__
#endif

#ifndef typeof
#define typeof __typeof__
#endif

#ifdef __ARM_ARCH_ISA_A64
#define debugbreak() asm volatile ("brk 0xf000")
#elif __x86_64__
#define debugbreak() asm volatile ("int3; nop")
#else
#error Unsupported Platform!
#endif

#ifdef _DEBUG
#define ASSERT(c) do { if (!(c)) debugbreak(); } while (0)
#define DEBUG_EXPORT
#else
#define ASSERT(c) do { (void)(c); } while (0)
#define DEBUG_EXPORT static
#endif

typedef float     f32;
typedef double    f64;
typedef uint8_t   u8;
typedef uint32_t  b32;
typedef uint32_t  u32;
typedef int32_t   i32;
typedef uint64_t  u64;
typedef int64_t   i64;
typedef ptrdiff_t size;
typedef size_t    usize;

typedef struct { u8 *beg, *end; } Arena;

#define s8(s)  (s8){.len = sizeof(s) - 1,  .data = (u8 *)s}
typedef struct { size len; u8 *data; } s8;

typedef struct {
	u8  *buffer;
	i32  capacity;
	i32  write_index;
	b32  errors;
} Stream;

typedef struct {
	void   *user_data;
	void   *arg;
	char   *fmt;
	char    data[BLOCKLEN];
	size    len;
} Block;

#define BLOCK_INIT_FN(name)   void name(Block *b, i32 block_index, Arena *a)
#define BLOCK_UPDATE_FN(name) b32  name(Block *b, f32 dt)
typedef BLOCK_UPDATE_FN(block_update_fn);

typedef struct FileWatch {
	struct FileWatch *next;
	block_update_fn  *update_fn;
	char             *path;
	i32               block_index;
	i32               wd;
} FileWatch;

/* TODO(rnp): replace this with arena usage */
static char       buffer[KB(1)];
static Stream     statusline;
static void      *display;
static i32        dirty_block_index = -1;
static FileWatch  file_watches;

static i32        dflag;

static void
die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);

	_exit(1);
}

static f64
get_time(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	f64 result = t.tv_sec + ((f64)t.tv_nsec) * 1e-9;
	return result;
}

static b32
timer_update(f32 *timer, f32 interval, f32 dt)
{
	b32 result = 0;
	*timer -= dt;
	while (*timer < 0) { *timer += interval; result = 1; }
	return result;
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

static void *
mem_clear(void *_p, u8 c, usize size)
{
	u8 *p = _p;
	for (usize i = 0; i < size; i++)
		p[i] = c;
	return p;
}

#define push_struct(a, t) alloc(a, t, 1)
#define alloc(a, t, n)  (t *)alloc_(a, sizeof(t), _Alignof(t), n)
static void *
alloc_(Arena *a, size len, size align, size count)
{
	size padding   = -(uintptr_t)a->beg & (align - 1);
	size available = a->end - a->beg - padding;
	if (available <= 0 || available / len < count) {
		ASSERT(0);
	}

	void *p  = a->beg + padding;
	a->beg  += padding + count * len;
	return mem_clear(p, 0, count * len);
}

static Stream
stream_alloc(Arena *a, size capacity)
{
	Stream result   = {0};
	result.buffer   = alloc(a, u8, capacity);
	result.capacity = capacity;
	return result;
}

static char *
stream_ensure_c_str(Stream *s)
{
	ASSERT(s->write_index < s->capacity);
	s->buffer[s->write_index] = 0;
	return (char *)s->buffer;
}

static void
stream_push_s8(Stream *s, s8 str)
{
	s->errors |= s->capacity <= (s->write_index + str.len);
	if (!s->errors) {
		memcpy(s->buffer + s->write_index, str.data, str.len);
		s->write_index += str.len;
	}
}

static void
add_file_watch(Arena *a, char *path, i32 block_index, block_update_fn *update_fn)
{
	i32 wd = inotify_add_watch(file_watches.wd, path, IN_CLOSE_WRITE|IN_MODIFY);
	if (wd != -1) {
		/* TODO(rnp): we will need to include inodes if we are expecting to watch
		 * files that could be removed */
		FileWatch *fw   = push_struct(a, FileWatch);
		fw->wd          = wd;
		fw->path        = path;
		fw->block_index = block_index;
		fw->update_fn   = update_fn;
		SLLPush(&file_watches, fw);
	}
}

#include "config.h"

#define X(name, format, argument) {.fmt = format, .arg = argument},
static Block blocks[] = {
	BLOCKS
};
#undef X

#if TICK_RATE_NANOSECONDS > 999999999
#error TICK_RATE_NANOSECONDS must not exceed 999 999 999
#endif

static void
terminate(int signo)
{
	if (!dflag) {
		XStoreName(display, DefaultRootWindow(display), 0);
		XCloseDisplay(display);
	}
	_exit(0);
}

static void
dispatch_file_watch_events(Arena a)
{
	u8 *mem = alloc_(&a, 4096, 64, 1);
	for (;;) {
		size rlen = read(file_watches.wd, mem, 4096);
		if (rlen <= 0)
			break;

		struct inotify_event *ie;
		for (u8 *data = mem; data < mem + rlen; data += sizeof(*ie) + ie->len) {
			ie = (void *)data;
			for (FileWatch *fw = file_watches.next; fw; fw = fw->next) {
				if (fw->wd != ie->wd)
					continue;

				b32 file_changed  = (ie->mask & IN_CLOSE_WRITE) != 0;
				file_changed     |= (ie->mask & IN_MODIFY)      != 0;
				/* TODO(rnp): it seems like this hits multiple times per update */
				if (file_changed && fw->update_fn(blocks + fw->block_index, 0)) {
					/* TODO(rnp): there might be an ordering issue here */
					if (dirty_block_index == -1 || fw->block_index < dirty_block_index)
						dirty_block_index = fw->block_index;
				}
			}
		}
	}
}

static void
update_status(void)
{
	statusline.write_index = 0;
	i32 block_index;
	for (block_index = 0; block_index < dirty_block_index; block_index++)
		statusline.write_index += blocks[block_index].len;

	for (; block_index < ARRAY_COUNT(blocks); block_index++) {
		Block *b = blocks + block_index;
		memcpy(statusline.buffer + statusline.write_index, b->data, b->len);
		statusline.write_index += b->len;
	}
	statusline.buffer[statusline.write_index] = 0;

	if (dflag) {
		puts((char *)statusline.buffer);
	} else {
		XStoreName(display, DefaultRootWindow(display), (char *)statusline.buffer);
		XSync(display, 0);
	}
}

static void
update_blocks(f32 dt)
{
	i32 count = 0;
	#define X(name, fmt, args) if (name ##_update(blocks + count++, dt)) dirty_block_index = count - 1;
	BLOCKS
	#undef X
}

static void
status_init(Arena *a)
{
	if (!dflag && !(display = XOpenDisplay(0)))
		die("XOpenDisplay: can't open display\n");

	statusline = stream_alloc(a, 4096);

	file_watches.wd = inotify_init1(O_NONBLOCK|O_CLOEXEC);

	i32 count = 0;
	#define X(name, fmt, arg) name ##_init(blocks + count, count, a); count++;
	BLOCKS
	#undef X

	struct sigaction sa;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = terminate;
	sigaction(SIGHUP,  &sa, 0);
	sigaction(SIGINT,  &sa, 0);
	sigaction(SIGTERM, &sa, 0);

	update_status();
}

static void
status_loop(Arena a)
{
	fd_set rfd;
	f64 last_time = get_time();

	for (;;) {
		dirty_block_index = -1;
		struct timespec t = {.tv_sec = TICK_RATE_SECONDS, .tv_nsec = TICK_RATE_NANOSECONDS};
		FD_ZERO(&rfd);
		FD_SET(file_watches.wd, &rfd);

		pselect(file_watches.wd + 1, &rfd, 0, 0, &t, 0);
		if (FD_ISSET(file_watches.wd, &rfd))
			dispatch_file_watch_events(a);

		f64 current_time = get_time();
		f32 dt = current_time - last_time;
		last_time = current_time;

		update_blocks(dt);
		if (dirty_block_index >= 0)
			update_status();
	}
}

static Arena
get_arena(void)
{
	static u8 memory[KB(64)];
	Arena a = {0};
	a.beg = memory;
	asm("" : "+r"(a.beg));
	a.end = a.beg + sizeof(memory);
	return a;
}

i32
main(i32 argc, char *argv[])
{
	char *argv0 = *argv;
	for (argv++; --argc && *argv && argv[0][0] == '-' && argv[0][1]; argv++) {
		if (argv[0][1] == '-' && argv[0][2] == '\0') {
			argv++;
			argc--;
			break;
		}

		for (i32 i = 1; argv[0][i]; i++) {
			switch (argv[0][i]) {
			case 'd': dflag = 1; break;
			default:  die("usage: %s [-d]\n", argv0);
			}
		}
	}

	Arena memory = get_arena();
	status_init(&memory);
	status_loop(memory);

	return 0;
}
