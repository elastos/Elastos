include environ/$(HOST)-$(ARCH).mk

PACKAGE_NAME   = libconfig-1.7.2.tar.gz
PACKAGE_URL    = https://github.com/hyperrealm/libconfig/archive/v1.7.2.tar.gz
SRC_DIR        = $(DEPS_DIR)/libconfig-1.7.2

CONFIG_COMMAND = $(shell scripts/libconfig.sh "command" $(HOST) $(ARCH) $(HOST_COMPILER))
CONFIG_OPTIONS = --prefix=$(DIST_DIR) \
        --enable-shared=no \
        --disable-shared \
        --enable-static \
        --with-pic=yes \
        --disable-cxx

define configure
    if [ ! -e $(SRC_DIR)/configure ]; then \
        cd $(SRC_DIR) && aclocal --force; \
        cd $(SRC_DIR) && autoheader --force; \
        cd $(SRC_DIR) && automake --add-missing; \
        cd $(SRC_DIR) && autoconf --force; \
    fi
    cd $(SRC_DIR) && CFLAGS="$(CFLAGS) -fvisibility=hidden" $(CONFIG_COMMAND) $(CONFIG_OPTIONS)
endef

define compile
    cd $(SRC_DIR) && CFLAGS="$(CFLAGS) -fvisibility=hidden" make
endef

include modules/rules.mk
