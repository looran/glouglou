#CFLAGS += -Wall -O2 -fPIC -shared -g
CFLAGS += -Wall -O0 -fPIC -shared -g

LIBDIR=/var/lib/glougloud/chroot/modules/
LIBNAME=glougloud_mod_1_net

TARGET  = ${LIBNAME}.so
SOURCES = glougloud_mod_1_net.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

install:
	@echo "installation of $(LIBNAME)"
	mkdir -p $(LIBDIR)
	install -m 0644 $(TARGET) $(LIBDIR)

clean:
	rm -f $(TARGET) $(OBJECTS)
