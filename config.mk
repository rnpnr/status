SRC =\
	blocks/gettime.c\
	blocks/linux/battery.c\
	blocks/linux/blight.c\
	blocks/linux/volume.c\
	blocks/mpd.c

PREFIX = /

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

INCS = -I$(X11INC) -I/usr/local/include
LIBS = -L$(X11LIB) -L/usr/local/lib -lmpdclient -lX11 -lasound

CPPFLAGS = -D_POSIX_C_SOURCE
CFLAGS = -O2 -std=c99 -Wall -pedantic $(CPPFLAGS) $(INCS)
LDFLAGS = $(LIBS)
