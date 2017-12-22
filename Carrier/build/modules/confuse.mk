include environ/$(HOST)-$(ARCH).mk

PACKAGE_NAME   = libconfuse-3.0.tar.gz
PACKAGE_URL    = https://github.com/martinh/libconfuse/archive/v3.0.tar.gz
SRC_DIR        = $(DEPS_DIR)/libconfuse-3.0

CONFIG_COMMAND = $(shell scripts/confuse.sh "command" $(HOST) $(ARCH) $(HOST_COMPILER))
CONFIG_OPTIONS = --prefix=$(DIST_DIR) \
	    --enable-shared=no \
	    --disable-shared \
	    --enable-static=yes \
	    --disable-nls \
	    --disable-rpath \
	    --without-libiconv \
	    --without-libintl

# run command if need on mac: "brew link --force gettext"
define configure
    if [ ! -e $(SRC_DIR)/configure ]; then \
        cd $(SRC_DIR) && ./autogen.sh; \
    fi
    cd $(SRC_DIR) && $(CONFIG_COMMAND) $(CONFIG_OPTIONS)
endef

include modules/rules.mk

