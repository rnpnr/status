SRC =\
	blocks/date.c\
	blocks/linux/battery.c\
	blocks/linux/blight.c\
	blocks/script.c
# blocks/linux/volume.c: needs: -lasound
#	blocks/linux/volume.c
# blocks/mpd.c: needs: -lmpdclient
#	blocks/mpd.c

PREFIX = /

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

INCS = -I$(X11INC) -I/usr/local/include
LIBS = -L$(X11LIB) -L/usr/local/lib -lX11

CPPFLAGS = -D_POSIX_C_SOURCE=200809L
CFLAGS = -O2 -std=c99 -Wall -pedantic
LDFLAGS = -s
