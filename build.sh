#!/bin/sh

cflags="-O3 -std=c99 -Wall -pedantic"
cflags="$cflags -D_XOPEN_SOURCE=500"
cflags="$cflags -I /usr/X11R6/include"

ldflags="-lX11"
#ldflags="$ldflags -lmpdclient" # needed for blocks/mpd.c
#ldflags="$ldflags -lasound"    # needed for blocks/linux/volume.c

echo "cflags:  $cflags"
echo "ldflags: $ldflags"

cc $cflags status.c $ldflags -o status
