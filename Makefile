include config.mk

SRC = status.c linux.c
OBJ = $(SRC:.c=.o)

all: status

config.h:
	cp config.def.h config.h

.c.o:
	$(CC) $(CFLAGS) -c $<

$(OBJ): config.h

status: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f *.o status

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f status $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/status

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/status
