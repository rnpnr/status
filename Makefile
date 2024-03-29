# See LICENSE for license details.
include config.mk

SRC += status.c util.c
OBJ = $(SRC:.c=.o)

all: status

config.h:
	cp config.def.h config.h

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCS) -o $@ -c $<

$(OBJ): config.h

status: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS) $(LDFLAGS)

clean:
	rm -f $(OBJ) status

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f status $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/status

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/status
