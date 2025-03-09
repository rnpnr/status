/* See LICENSE for license details. */
#include <fcntl.h>
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

#define ABS(a)           ((a) < 0 ? -(a) : (a))
#define MIN(a, b)        ((a) < (b) ? (a) : (b))
#define BETWEEN(x, a, b) ((x) >= (a) && (x) <= (b))
#define ARRAY_COUNT(a)   (sizeof(a) / sizeof(*a))

#define I64_MAX  INT64_MAX
#define BLOCKLEN 128

#define ISSPACE(c)     ((c) == ' ' || (c) == '\n' || (c) == '\t')

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
	f32     timer;
	u32     len;
	char    data[BLOCKLEN];
} Block;

#define BLOCK_INIT_FN(name)   void name(Block *b, i32 block_index, Arena *a)
#define BLOCK_UPDATE_FN(name) void name(Block *b)
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

static s8
read_s8(char *path, s8 buffer)
{
	i32 fd     = open(path, O_RDONLY);
	buffer.len = read(fd, buffer.data, buffer.len);
	close(fd);
	return buffer;
}

static i64
read_i64(char *path)
{
	char buffer[64];
	s8 s = read_s8(path, (s8){.len = sizeof(buffer), .data = (u8 *)buffer});

	i64 result = 0;
	i64 i      = s.len && (s.data[0] == '-' || s.data[0] == '+');
	i64 sign   = (s.len && s.data[0] == '-') ? -1 : 1;

	for (; i < s.len && BETWEEN(s.data[i], '0', '9'); i++) {
		i32 digit = (i32)s.data[i] - '0';
		if (result > (I64_MAX - digit) / 10) {
			result = I64_MAX;
		} else {
			result = 10 * result + digit;
		}
	}

	return sign * result;
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

static b32
s8_equal(s8 a, s8 b)
{
	b32 result = a.len == b.len;
	if (result) {
		for (size i = 0; i < a.len; i++)
			result &= a.data[i] == b.data[i];
	}
	return result;
}

static s8
s8_trim_space(s8 a)
{
	while (a.len > 0 && ISSPACE(a.data[0]))         { a.len--; a.data++; }
	while (a.len > 0 && ISSPACE(a.data[a.len - 1])) { a.len--; }
	return a;
}

static void
stream_reset(Stream *s, size position)
{
	s->errors = !BETWEEN(position, 0, s->capacity);
	if (!s->errors) s->write_index = position;
}

static Stream
stream_alloc(Arena *a, size capacity)
{
	Stream result   = {0};
	result.buffer   = alloc(a, u8, capacity);
	result.capacity = capacity;
	return result;
}

static void
stream_force_push_byte(Stream *s, u8 byte)
{
	s->errors |= s->capacity < (s->write_index + 1);
	if (s->errors) s->buffer[s->capacity - 1]  = byte;
	else           s->buffer[s->write_index++] = byte;
}

static char *
stream_ensure_c_str(Stream *s)
{
	stream_force_push_byte(s, 0);
	return (char *)s->buffer;
}

static void
stream_push(Stream *s, void *data, size len)
{
	s->errors |= s->capacity <= (s->write_index + len);
	if (!s->errors) {
		memcpy(s->buffer + s->write_index, data, len);
		s->write_index += len;
	}
}

static void
stream_push_s8(Stream *s, s8 str)
{
	stream_push(s, str.data, str.len);
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

#define X(name, format, interval, argument) {.fmt = format, .arg = argument},
static Block blocks[] = {
	BLOCKS
};
#undef X

#if TICK_RATE_NANOSECONDS > 999999999
#error TICK_RATE_NANOSECONDS must not exceed 999 999 999
#endif

static void
terminate(i32 signo)
{
	if (!dflag) {
		XStoreName(display, DefaultRootWindow(display), 0);
		XCloseDisplay(display);
	}
	_exit(0);
}

static void
update_dirty_block_index(i32 new_index)
{
	if (dirty_block_index == -1 || new_index < dirty_block_index)
		dirty_block_index = new_index;
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
				if (file_changed) {
					fw->update_fn(blocks + fw->block_index);
					update_dirty_block_index(fw->block_index);
				}
			}
		}
	}
}

static void
update_status(void)
{
	stream_reset(&statusline, 0);
	i32 block_index;
	for (block_index = 0; block_index < dirty_block_index; block_index++)
		statusline.write_index += blocks[block_index].len;

	for (; block_index < ARRAY_COUNT(blocks); block_index++) {
		Block *b = blocks + block_index;
		stream_push(&statusline, b->data, b->len);
	}

	if (dflag) {
		stream_force_push_byte(&statusline, '\n');
		write(STDOUT_FILENO, statusline.buffer, statusline.write_index);
	} else {
		XStoreName(display, DefaultRootWindow(display),
		           stream_ensure_c_str(&statusline));
		XSync(display, 0);
	}
}

static b32
timer_update(f32 *timer, f32 interval, f32 dt)
{
	b32 result = dt <= 0;
	if (interval > 0) {
		*timer -= dt;
		while (*timer < 0) { *timer += interval; result = 1; }
	}
	return result;
}

static void
update_blocks(f32 dt)
{
	i32 count = 0;
	#define X(name, fmt, interval, args) \
		if (timer_update(&blocks[count].timer, interval, dt)) { \
			name ##_update(blocks + count);                 \
			update_dirty_block_index(count);                \
		}                                                       \
		count++;
	BLOCKS
	#undef X
}

static void
reload_all_blocks(i32 _unused)
{
	update_blocks(0);
}

static void
status_init(Arena *a)
{
	if (!dflag && !(display = XOpenDisplay(0)))
		die("XOpenDisplay: can't open display\n");

	statusline = stream_alloc(a, 4096);

	file_watches.wd = inotify_init1(O_NONBLOCK|O_CLOEXEC);

	i32 count = 0;
	#define X(name, fmt, interval, arg) name ##_init(blocks + count, count, a); count++;
	BLOCKS
	#undef X

	struct sigaction sa;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = terminate;
	sigaction(SIGINT,  &sa, 0);
	sigaction(SIGTERM, &sa, 0);

	sa.sa_handler = reload_all_blocks;
	sigaction(SIGHUP, &sa, 0);

	sa.sa_flags   = 0;
	sa.sa_handler = SIG_IGN;
	for (size i = SIGRTMIN; i <= SIGRTMAX; i++)
		sigaction(i, &sa, 0);

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

	/* NOTE(rnp): fork ourselves to the background and run as a daemon */
	if (!dflag) {
		switch(fork()) {
		case -1: die("failed to fork to background\n");
		case  0: setsid(); break;
		default: _exit(0);
		}
	}

	Arena memory = get_arena();
	status_init(&memory);
	status_loop(memory);

	return 0;
}
