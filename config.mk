PREFIX = /

LIBS = -lmpdclient -lX11

CFLAGS = -O2 -std=c99 -Wall -pedantic
LDFLAGS = $(LIBS)
