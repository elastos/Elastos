PREFIX ?= /usr/local
INCLUDE_PATH ?= include
LIBRARY_PATH ?= lib

INSTALL_INCLUDE_PATH = $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH = $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)

INSTALL ?= cp -a

CFLAGS := $(CFLAGS) -I../../src/common -I$(INSTALL_INCLUDE_PATH) -fPIC -Wall  -Wstrict-prototypes -Wwrite-strings

LIBS = -lelacommon -lelacarrier -lelasession -lsodium -lconfig -lpthread

OBJS = pfd.o config.o

.PHONY: all clean install

all: elapfd

elapfd: $(OBJS)
	$(CC) -o $@ $(OBJS) -L$(INSTALL_LIBRARY_PATH) $(LIBS)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

install: all

clean:
	rm -rf elapfd *.o
