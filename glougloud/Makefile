PROG = glougloud
HEADERS = glougloud.h
SOURCES = glougloud.c  probes.c  redis.c  viz.c
OBJECTS = $(SOURCES:.c=.o)
CFLAGS+=-Wall -g
LDFLAGS=-levent -ldnet -lglouglou -lsendbuf -ldl -lhiredis
USER_PROBES = _ggdprobe
USER_VIZ = _ggdviz
GLOUGLOUD_HOME = "/var/lib/glougloud"
GLOUGLOUD_CHROOT = "$(GLOUGLOUD_HOME)/chroot"

PREFIX=/usr/local
INCLUDEDIR=$(PREFIX)/include
BINDIR=$(PREFIX)/sbin

all:
	make $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG) $(LDFLAGS)
	make -C glougloud_mod_0_internal/
	make -C glougloud_mod_1_net/

install: $(PROG)
	@echo "creating 2 users: $(USER_PROBES), $(USER_VIZ)"
	cmdp="useradd -r -d $(GLOUGLOUD_CHROOT) -s /sbin/nologin $(USER_VIZ)" ;\
	echo $$cmdp; $$($$cmdp) ;\
	cmdp="useradd -r -d $(GLOUGLOUD_CHROOT) -g $(USER_VIZ) -s /sbin/nologin $(USER_PROBES)" ;\
	echo $$cmdp; $$($$cmdp) ;\
	echo done
	@echo "creating home $(GLOUGLOUD_HOME)"
	sudo mkdir -p $(GLOUGLOUD_HOME)
	chmod 755 $(GLOUGLOUD_HOME)
	@echo "creating chroot $(GLOUGLOUD_CHROOT)"
	sudo mkdir -p $(GLOUGLOUD_CHROOT)
	chmod 755 $(GLOUGLOUD_CHROOT)
	sudo mkdir -p $(GLOUGLOUD_CHROOT)/socket
	chown $(USER_PROBES):$(USER_VIZ) $(GLOUGLOUD_CHROOT)/socket
	chmod 750 $(GLOUGLOUD_CHROOT)/socket
	@echo done
	@echo "installation of $(PROG)"
	mkdir -p $(BINDIR)
	mkdir -p $(INCLUDEDIR)
	install -m 0755 $(PROG) $(BINDIR)
	install -m 0644 $(HEADERS) $(INCLUDEDIR)
	make -C glougloud_mod_0_internal_headers/ install
	make -C glougloud_mod_0_internal/ install
	make -C glougloud_mod_1_net_headers/ install
	make -C glougloud_mod_1_net/ install

clean:
	rm -f $(PROG) $(OBJECTS) *~
	make -C glougloud_mod_0_internal/ clean
	make -C glougloud_mod_1_net/ clean
