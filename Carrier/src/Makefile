PREFIX ?= /usr/local

PREFIX := $(abspath $(PREFIX))
HOST   ?= $(uname -s)
ARCH   ?= $(uname -m)

MODULES = common carrier session

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
