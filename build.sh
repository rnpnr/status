#!/bin/sh

cflags="-O3 -std=c99 -Wall -pedantic"
cflags="$cflags -D_POSIX_C_SOURCE=200808L"
cflags="$cflags -I /usr/X11R6/include"

ldflags="-lX11"
#ldflags="$ldflags -lmpdclient" # needed for blocks/mpd.c
#ldflags="$ldflags -lasound"    # needed for blocks/linux/volume.c

echo "cflags:  $cflags"
echo "ldflags: $ldflags"

cc $cflags status.c $ldflags -o status
