LIBNAME = libelasession

PREFIX ?= /usr/local
INCLUDE_PATH ?= include
LIBRARY_PATH ?= lib
HOST ?= $(uname -s)

INSTALL_INCLUDE_PATH = $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH = $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)

INSTALL ?= cp -a

CFLAGS := $(CFLAGS) -I. -I../common -I$(INSTALL_INCLUDE_PATH) -DCARRIER_BUILD -fPIC -Wall  -Wstrict-prototypes -Wwrite-strings
LIBS   := -lelacommon -lelacarrier -lpjmedia -lpjnath -lpjlib-util -lpj -lm

ifeq ($(HOST), Android)
	LIBS := $(LIBS) -llog
else
ifeq (, $(findstring $(HOST), Darwin iOS))
	LIBS := $(LIBS) -lpthread
endif
endif

ifeq ($(HOST), iOS)
        LIBS := $(LIBS) -framework CoreFoundation -framework CFNetwork
endif

STLIBNAME = $(LIBNAME).a

ifneq (, $(findstring $(HOST), Darwin iOS))
	DYLIBNAME = $(LIBNAME).dylib
else
	DYLIBNAME = $(LIBNAME).so
endif

PSEUDOTCP_SRCS = pseudotcp/pseudotcp.c pseudotcp/glist.c pseudotcp/gqueue.c
SRCS = session.c crypto_handler.c reliable_handler.c multiplex_handler.c portforwarding.c fdset.c udp_eventfd.c  ice.c $(PSEUDOTCP_SRCS)

OBJS = $(SRCS:.c=.o)

LOBJS = $(SRCS:.c=.lo)

.SUFFIXES:
.SUFFIXES: .c .lo .o

HEADERS = ela_session.h

.PHONY: all clean install

all: $(DYLIBNAME) $(STLIBNAME)

$(DYLIBNAME): $(OBJS)
	$(CC) -shared $(LDFLAGS) -o $@ $(OBJS) -L$(INSTALL_LIBRARY_PATH) $(LIBS)

$(STLIBNAME): $(LOBJS)
	$(AR) rcs $@ $^

.c.o:
	$(CC) -c $(CFLAGS) -DCARRIER_DYNAMIC -o $@ $<

.c.lo:
	$(CC) -c $(CFLAGS) -DCARRIER_STATIC -o $@ $<

pseudotcp/pseudotcp.o: pseudotcp/pseudotcp.c
	$(CC) -c $(CFLAGS) -Wno-absolute-value -o $@ $<

pseudotcp/pseudotcp.lo: pseudotcp/pseudotcp.c
	$(CC) -c $(CFLAGS) -Wno-absolute-value -o $@ $<

install: all
	mkdir -p $(INSTALL_LIBRARY_PATH) $(INSTALL_INCLUDE_PATH)
	$(INSTALL) $(HEADERS) $(INSTALL_INCLUDE_PATH)
	$(INSTALL) $(DYLIBNAME) $(INSTALL_LIBRARY_PATH)
	$(INSTALL) $(STLIBNAME) $(INSTALL_LIBRARY_PATH)

clean:
	rm -rf $(DYLIBNAME) $(STLIBNAME) 
	rm -rf *.o *.lo
	rm -rf pseudotcp/*.o pseudotcp/*.lo

