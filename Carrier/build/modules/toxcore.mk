include environ/$(HOST)-$(ARCH).mk

PACKAGE_NAME   = c-toxcore-0.1.10.tar.gz
PACKAGE_URL    = https://github.com/TokTok/c-toxcore/releases/download/v0.1.10/$(PACKAGE_NAME)
SRC_DIR        = $(DEPS_DIR)/c-toxcore-0.1.10

CONFIG_COMMAND = $(shell scripts/toxcore.sh "command" $(HOST) $(ARCH) $(HOST_COMPILER))
CONFIG_OPTIONS = --prefix=$(DIST_DIR) \
        --with-dependency-search=$(DIST_DIR) \
        --enable-static \
        --disable-shared \
        --disable-ntox \
        --disable-daemon \
        --disable-tests \
        --disable-testing \
        --disable-av

define configure
    $(BUILD_DIR)/patch/toxcore.sh $(SRC_DIR)
    if [ ! -e $(SRC_DIR)/configure ]; then \
        cd $(SRC_DIR) && ./autogen.sh; \
    fi
    cd $(SRC_DIR) && CFLAGS="${CFLAGS} -fvisibility=hidden -DELASTOS_BUILD" $(CONFIG_COMMAND) $(CONFIG_OPTIONS)
endef

include modules/rules.mk


