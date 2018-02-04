PREFIX ?= /usr/local
INCLUDE_PATH ?= include
LIBRARY_PATH ?= lib
HOST ?= $(uname -s)

INSTALL_INCLUDE_PATH = $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH = $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)

INSTALL ?= cp -a

CFLAGS := $(CFLAGS) -I.. \
        -I. \
        -I../../src/common \
        -I../../src/carrier \
        -I../../src/session \
        -I$(INSTALL_INCLUDE_PATH) \
        -Wall -Wstrict-prototypes -Wwrite-strings

ifneq (,$(findstring $(HOST), Linux Android))
	CFLAGS := $(CFLAGS) -Wno-discarded-qualifiers
endif

ifneq (, $(findstring $(HOST), Darwin iOS))
	CFLAGS := $(CFLAGS) -Wno-incompatible-pointer-types-discards-qualifiers
endif

LDFLAGS := $(LDFLAGS) \
          -L../../src/common \
          -L../../src/session

LIBS   := -lelacommon -lelacarrier -lelasession -lcunit -lconfuse -lncurses -lm

ifeq ($(HOST), Android)
	LIBS := $(LIBS) -llog
else
ifeq (, $(findstring $(HOST), Darwin, iOS))
	LIBS := $(LIBS) -lpthread
endif
endif

CARRIER_TESTS=$(shell cat carrier/suites.h | grep ^'DECL_TESTSUITE' | awk -F '(' '{print $$2}' | awk -F ')' '{print $$1}' | tr '\n\r' ' ')
CARRIER_TESTS_OBJS =$(addprefix carrier/,$(addsuffix .o,$(CARRIER_TESTS)))

SESSION_TESTS=$(shell cat session/suites.h | grep ^'DECL_TESTSUITE' | awk -F '(' '{print $$2}' | awk -F ')' '{print $$1}' | tr '\n\r' ' ')
SESSION_TESTS_OBJS =$(addprefix session/,$(addsuffix .o,$(SESSION_TESTS)))

ROBOT_OBJS = robot/robot.o  robot/cmd.o

OBJS = tests.o test_helper.o launcher.o config.o $(ROBOT_OBJS) $(CARRIER_TESTS_OBJS) $(SESSION_TESTS_OBJS)

tests.o: suites.h carrier/suites.h session/suites.h

.DUMMY: all

all: install

install: api_tests
	@echo "Dummy make install."

api_tests: $(OBJS)
	$(CC) -o $@ $^ -L$(INSTALL_LIBRARY_PATH) $(LDFLAGS) $(LIBS)

test: api_tests
	LD_LIBRARY_PATH=$(INSTALL_LIBRARY_PATH) ./api_tests

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf api_tests *.o
	rm -f carrier/*.o
	rm -f session/*.o
	rm -f robot/*.o
	rm -f *.log
	rm -rf .tests
	rm -rf .robot
