#!/bin/sh

cflags="-march=native -O3 -std=c11 -Wall -pedantic"
cflags="$cflags -D_XOPEN_SOURCE=700"
cflags="$cflags -I /usr/X11R6/include"
#cflags="${cflags} -O0 -ggdb -D_DEBUG"
#cflags="${cflags} -fsanitize=address,undefined"

ldflags="-lX11"
#ldflags="$ldflags -lmpdclient" # needed for blocks/mpd.c
#ldflags="$ldflags -lasound"    # needed for blocks/linux/volume.c

#echo "cflags:  $cflags"
#echo "ldflags: $ldflags"

cc $cflags status.c $ldflags -o status
