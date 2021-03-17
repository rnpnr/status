# See LICENSE for license details.
include config.mk

SRC =\
	blocks/battery.c\
	blocks/gettime.c\
	blocks/mpd.c\
	blocks/volume.c\
	status.c util.c
OBJ = $(SRC:.c=.o)

all: status

config.h:
	cp config.def.h config.h

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ): config.h

status: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f blocks/*.o *.o status

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f status $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/status

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/status
