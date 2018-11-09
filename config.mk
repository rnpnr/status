PREFIX = /

LIBS = -lmpdclient -lX11

CPPFLAGS = -D_POSIX_C_SOURCE
CFLAGS = -O2 -std=c99 -Wall -pedantic $(CPPFLAGS)
LDFLAGS = $(LIBS)
