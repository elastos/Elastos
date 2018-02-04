PREFIX ?= /usr/local

PREFIX := $(abspath $(PREFIX))
HOST   ?= $(uname -s)
ARCH   ?= $(uname -m)

MODULES =

ifneq (, $(findstring $(HOST)-$(ARCH), Darwin-x86_64 Linux-x86_64 Raspbian-armv7l))
MODULES = api 
endif

all: install

install:
	@for i in $(MODULES) ; do \
	(cd $$i && echo "making in $$i..." && \
	PREFIX=$(PREFIX) $(MAKE) install) || exit 1; \
	done;

clean:
	@for i in $(MODULES) ; do \
	(cd $$i && echo "making in $$i..." && \
	PREFIX=$(PREFIX) $(MAKE) clean) || exit 1; \
	done;
