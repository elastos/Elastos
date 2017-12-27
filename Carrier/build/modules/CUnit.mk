include environ/$(HOST)-$(ARCH).mk

PACKAGE_NAME   = CUnit-2.1-3.tar.bz2
PACKAGE_URL    = http://jaist.dl.sourceforge.net/project/cunit/CUnit/2.1-3/$(PACKAGE_NAME)
SRC_DIR        = $(DEPS_DIR)/CUnit-2.1-3

CONFIG_COMMAND = $(shell ./scripts/CUnit.sh "command" $(HOST) $(ARCH) $(HOST_COMPILER))
CONFIG_OPTIONS = --prefix=$(DIST_DIR)

define configure
    if [ ! -e $(SRC_DIR)/configure ]; then \
        cd $(SRC_DIR) && aclocal && autoconf && autoreconf -if; \
    fi

    cd $(SRC_DIR) && $(CONFIG_COMMAND) $(CONFIG_OPTIONS)
endef

define make-clean
    cd $(SRC_DIR) && make clean-all
endef

define config-clean
    for c in "clean" "distclean"; do \
        cd $(SRC_DIR) && make $$c-all; \
    done;
endef

include modules/rules.mk

