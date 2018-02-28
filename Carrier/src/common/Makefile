LIBNAME = libelacommon

PREFIX ?= /usr/local
INCLUDE_PATH ?= include
LIBRARY_PATH ?= lib
HOST ?= $(uname -s)

INSTALL_INCLUDE_PATH = $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH = $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)

INSTALL ?= cp -a

CFLAGS := $(CFLAGS) -I. -I$(INSTALL_INCLUDE_PATH) -DCOMMON_BUILD -DSODIUM_STATIC -fvisibility=hidden -fPIC -Wall  -Wstrict-prototypes -Wwrite-strings
LDFLAGS := $(LDFLAGS) -fvisibility=hidden

LIBS   := -lsodium

ifneq (,$(findstring $(HOST), Darwin iOS))
	CFLAGS := $(CFLAGS) -Wno-incompatible-pointer-types-discards-qualifiers
else
ifneq (, $(findstring $(HOST), Android))
	CFLAGS := $(CFLAGS) -Iandroid
        LDFLAGS := $(LDFLAGS) -llog
endif
endif

ifeq (, $(findstring $(HOST), Android Darwin iOS))
    LIBS := $(LIBS) -lpthread
endif

STLIBNAME = $(LIBNAME).a

ifneq (, $(findstring $(HOST), Darwin iOS))
	DYLIBNAME = $(LIBNAME).dylib
else
	DYLIBNAME = $(LIBNAME).so
endif

SRCS = vlog.c base58.c rc_mem.c linkedhashtable.c linkedlist.c crypto.c time_util.c bitset.c ids_heap.c timerheap.c socket.c

ifneq (,$(findstring $(HOST), Android))
	SRCS := $(SRCS) android/ifaddrs.c
endif

OBJS = $(SRCS:.c=.o)

LOBJS = $(SRCS:.c=.lo)

.SUFFIXES:
.SUFFIXES: .c .lo .o

.PHONY: all clean install

all: $(DYLIBNAME) $(STLIBNAME)

$(DYLIBNAME): $(OBJS)
	$(CC) -shared $(LDFLAGS) -o $@ $(OBJS) -L$(INSTALL_LIBRARY_PATH) $(LIBS)

$(STLIBNAME): $(LOBJS)
	$(AR) rcs $@ $(LOBJS)

.c.o:
	$(CC) -c $(CFLAGS) -DCOMMON_DYNAMIC -o $@ $<

.c.lo:
	$(CC) -c $(CFLAGS) -DCOMMON_STATIC -o $@ $<

install: all
	mkdir -p $(INSTALL_LIBRARY_PATH) $(INSTALL_INCLUDE_PATH)
	$(INSTALL) $(DYLIBNAME) $(INSTALL_LIBRARY_PATH)
	$(INSTALL) $(STLIBNAME) $(INSTALL_LIBRARY_PATH)

clean:
	rm -rf $(DYLIBNAME) $(STLIBNAME) *.o *.lo android/*.o android/*.lo
