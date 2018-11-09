include config.mk

SRC = status.c
OBJ = $(SRC:.c=.o)

all: status

.c.o:
	$(CC) $(CFLAGS) -c $<

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
