include environ/$(HOST)-$(ARCH).mk

PACKAGE_NAME   = flatcc-0.5.0.tar.gz
PACKAGE_URL    = https://github.com/dvidelabs/flatcc/archive/v0.5.0.tar.gz
SRC_DIR        = $(DEPS_DIR)/flatcc-0.5.0
FLATCC_MK      = $(SRC_DIR)/flatcc.mk

define compile
    cd $(SRC_DIR) && PREFIX=$(DIST_DIR) make -f flatcc.mk
endef

define install
    cd $(SRC_DIR) && PREFIX=$(DIST_DIR) make -f flatcc.mk install
endef

define uninstall
    rm -rf $(DIST_DIR)/include/flatcc
    rm -f $(DIST_DIR)/lib/liblatcc.a
    rm -f $(DIST_DIR)/lib/liblatccrt.a
endef

define make-clean
    cd $(SRC_DIR) && PREFIX=$(DIST_DIR) make -f flatcc.mk clean
    @touch $(FLATCC_MK)
endef

define config-clean
    echo "Dummy cofig clean ..."
endef

define uninstall
    cd $(SRC_DIR) && PREFIX=$(DIST_DIR) make -f flatcc.mk uninstall
endef

include modules/rules.mk

$(COMPILE_STATUS): $(CONFIG_STATUS) $(FLATCC_MK)
	$(call compile)

################################################################################
define FLATCC_MK_CONTENT

PREFIX ?= /usr/local
INCLUDE_PATH ?= include
LIBRARY_PATH ?= lib
HOST_COMPILER = $(if $$(HOST),$$(HOST),$(uname -s))

INSTALL_INCLUDE_PATH = $$(DESTDIR)$$(PREFIX)/$$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH = $$(DESTDIR)$$(PREFIX)/$$(LIBRARY_PATH)

INSTALL ?= cp -a

CFLAGS := $$(CFLAGS) -Iinclude -Iconfig -Iexternal

R_CFLAGS = -fpic -fvisibility=hidden $$(CFLAGS) -Wall -Wno-strict-prototypes -Wwrite-strings -Wno-incompatible-pointer-types-discards-qualifiers

LIBFLATCCRT_OBJS = \
        src/runtime/builder.o \
        src/runtime/emitter.o \
        src/runtime/json_parser.o \
        src/runtime/json_printer.o \
        src/runtime/verifier.o

LIBFLATCC_OBJS = \
        external/hash/cmetrohash64.o \
        external/hash/str_set.o \
        external/hash/ptr_set.o \
        src/compiler/hash_tables/symbol_table.o \
        src/compiler/hash_tables/scope_table.o \
        src/compiler/hash_tables/name_table.o \
        src/compiler/hash_tables/schema_table.o \
        src/compiler/hash_tables/value_set.o \
        src/compiler/fileio.o \
        src/compiler/parser.o \
        src/compiler/semantics.o \
        src/compiler/coerce.o \
        src/compiler/flatcc.o \
        src/compiler/codegen_c.o \
        src/compiler/codegen_c_reader.o \
        src/compiler/codegen_c_sort.o \
        src/compiler/codegen_c_builder.o \
        src/compiler/codegen_c_verifier.o \
        src/compiler/codegen_c_json_parser.o \
        src/compiler/codegen_c_json_printer.o \
        src/runtime/builder.o \
        src/runtime/emitter.o

FLATCC_CLI_OBJS = \
        src/cli/flatcc_cli.o

.PHONY: all clean install uninstall

#all: libflatcc.a libflatccrt.a flatcc
all: libflatcc.a libflatccrt.a

libflatcc.a: $$(LIBFLATCC_OBJS)
	$$(AR) rcs $$@ $$(LIBFLATCC_OBJS)

libflatccrt.a: $$(LIBFLATCCRT_OBJS)
	$$(AR) rcs $$@ $$(LIBFLATCCRT_OBJS)

flatcc: src/cli/flatcc_cli.o libflatcc.a libflatccrt.a
	$$(CC) $$< -L. -L$$(INSTALL_LIBRARY_PATH) -lflatcc -lflatccrt -o $$@

.c.o:
	$$(CC) -I$$(INSTALL_INCLUDE_PATH) -c $$(R_CFLAGS) -o $$@ $$<

install: all
	mkdir -p $$(INSTALL_LIBRARY_PATH) $$(INSTALL_INCLUDE_PATH)
	$$(INSTALL) include/flatcc    $$(INSTALL_INCLUDE_PATH)/flatcc
	$$(INSTALL) libflatcc.a   $$(INSTALL_LIBRARY_PATH)
	$$(INSTALL) libflatccrt.a $$(INSTALL_LIBRARY_PATH)

clean:
	rm -rf external/hash/*.o
	rm -rf src/cli/*.o
	rm -rf src/runtime/*.o
	rm -rf src/compiler/*.o
	rm -rf src/compiler/hash_tables/*.o

uninstall:
	rm -rf $$(INSTALL_INCLUDE_PATH)/flatcc
	rm -rf $$(INSTALL_LIBRARY_PATH)/libflatcc.a
	rm -rf $$(INSTALL_LIBRARY_PATH)/libflatccrt.a

endef

export FLATCC_MK_CONTENT

$(FLATCC_MK): $(SOURCE_STATUS)
	@echo "$$FLATCC_MK_CONTENT" > $@

