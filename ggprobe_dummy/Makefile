PROG = ggprobe_dummy
SOURCES = ggprobe_dummy.c
OBJECTS = $(SOURCES:.c=.o)
CFLAGS+=-Wall -g
LDFLAGS=-levent -ldnet -lglouglou -lsendbuf -ldl -lhiredis

PREFIX=/usr/local
BINDIR=$(PREFIX)/sbin

all:
	make $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG) $(LDFLAGS)

install: $(PROG)
	@echo "installation of $(PROG)"
	mkdir -p $(BINDIR)
	install -m 0755 $(PROG) $(BINDIR)

clean:
	rm -f $(PROG) $(OBJECTS) *~
