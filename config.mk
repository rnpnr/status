PREFIX = /

LIBS = -lmpdclient -lX11 -lasound

CPPFLAGS = -D_POSIX_C_SOURCE
CFLAGS = -O2 -std=c99 -Wall -pedantic $(CPPFLAGS)
LDFLAGS = $(LIBS)
