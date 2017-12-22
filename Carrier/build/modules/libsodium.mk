include environ/$(HOST)-$(ARCH).mk

PACKAGE_NAME   = libsodium-1.0.11.tar.gz
PACKAGE_URL    = https://github.com/jedisct1/libsodium/releases/download/1.0.11/$(PACKAGE_NAME)
SRC_DIR        = $(DEPS_DIR)/libsodium-1.0.11

CONFIG_COMMAND = $(shell scripts/libsodium.sh "command" $(HOST) $(ARCH) $(HOST_COMPILER))
CONFIG_OPTIONS = --prefix=$(DIST_DIR) \
	    --enable-shared=no \
	    --disable-shared \
	    --enable-static \
	    --with-pic=yes \
	    --with-pthreads=yes

define configure
    cd $(SRC_DIR) && CFLAGS="$(CFLAGS) -fvisibility=hidden -DSODIUM_STATIC" $(CONFIG_COMMAND) $(CONFIG_OPTIONS)
endef

define compile
    cd $(SRC_DIR) && CFLAGS="$(CFLAGS) -fvisibility=hidden -DSODIUM_STATIC" make
endef

include modules/rules.mk

