#CFLAGS += -Wall -O2 -fPIC -shared -g
CFLAGS += -Wall -O0 -fPIC -shared -g

PREFIX=/usr/local
INCLUDEDIR=$(PREFIX)/include
LIBDIR=$(PREFIX)/lib
LIBNAME=libglouglou

TARGET  = ${LIBNAME}.so
SOURCES = log.c client.c packet.c server.c utils.c libglouglou_mod_0_internal/libglouglou_mod_0_internal.c
HEADERS = libglouglou.h libglouglou_mod_0_internal.h
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

install:
	@echo "installation of $(LIBNAME)"
	mkdir -p $(LIBDIR)
	mkdir -p $(INCLUDEDIR)
	install -m 0644 $(TARGET) $(LIBDIR)
	install -m 0644 $(HEADERS) $(INCLUDEDIR)

tests:
	$(MAKE) -C tests run

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: tests
