#CFLAGS += -Wall -O2 -fPIC -shared -g
CFLAGS += -Wall -O0 -fPIC -shared -g

PREFIX=/usr/local
INCLUDEDIR=$(PREFIX)/include
LIBDIR=$(PREFIX)/lib
LIBNAME=libgsnetstream

TARGET  = ${LIBNAME}.so
SOURCES = gsnetstream.c
HEADERS = gsnetstream.h
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
