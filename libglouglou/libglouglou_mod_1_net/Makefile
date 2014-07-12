# Glougloud module header generic Makefile

PREFIX=/usr/local
INCLUDEDIR=$(PREFIX)/include/glougloud/modules

HEADERS = $(shell echo *.h)

all:
	@echo "this package contains headers only,"
	@echo "run \"make install\" to install it."

install:
	@echo "headers installation"
	mkdir -p $(INCLUDEDIR)
	install -m 0644 $(HEADERS) $(INCLUDEDIR)

