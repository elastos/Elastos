SOURCE_STATUS  := $(SRC_DIR)/.source_status
CONFIG_STATUS  := $(SRC_DIR)/.config_status
COMPILE_STATUS := $(SRC_DIR)/.compile_status
INSTALL_STATUS := $(SRC_DIR)/.install_status

define def-source-fetch
    ./scripts/source.sh $(PACKAGE_URL) $(PACKAGE_NAME) $(REPO_DIR) $(DEPS_DIR)
endef

define def-configure
    @echo "Dummy configure ..."
endef

define def-compile
    cd $(SRC_DIR) && make
endef

define def-install
    cd $(SRC_DIR) && PREFIX=$(DIST_DIR) make install
endef

define def-uninstall
    cd $(SRC_DIR) && PREFIX=$(DIST_DIR) make uninstall
endef

define def-make-clean
    cd $(SRC_DIR) && make clean
endef

define def-config-clean
    for m in distclean clean; do \
        cd $(SRC_DIR) && make $$m; \
    done
endef

define def-source-clean
    rm -rf $(SRC_DIR)
endef

define def-dev-dist
    @echo "Dummy generate dev distribution ..."
endef

.PHONY: all
.PHONY: source config make install dist
.PHONY: uninstall clean config-clean source-clean
all: install

$(SOURCE_STATUS):
ifdef source-fetch
	$(call source-fetch)
else
	$(call def-source-fetch)
endif
	touch $@

$(CONFIG_STATUS): $(SOURCE_STATUS)
ifdef configure
	$(call configure)
else
	$(call def-configure)
endif
	touch $@

$(COMPILE_STATUS): $(CONFIG_STATUS)
ifdef compile 
	$(call compile) 
else
	$(call def-compile)
endif
	touch $@

$(INSTALL_STATUS): $(COMPILE_STATUS)
	mkdir -p $(DIST_DIR)
ifdef install
	$(call install)
else
	$(call def-install)
endif
	touch $@

dist: install
ifdef dev-dist
	$(call dev-dist)
else
	$(call def-dev-dist)
endif

install: $(INSTALL_STATUS)
make:    $(COMPILE_STATUS)
config:  $(CONFIG_STATUS)
source:  $(SOURCE_STATUS)

uninstall:
ifdef uninstall
	$(call uninstall)
else
	$(call def-uninstall)
endif
	touch $(COMPILE_STATUS)

clean:
ifdef make-clean
	$(call make-clean)
else
	$(call def-make-clean)
endif
	touch $(CONFIG_STATUS)

config-clean:
ifdef config-clean
	$(call config-clean)
else
	$(call def-config-clean)
endif
	touch $(SOURCE_STATUS)

source-clean:
ifdef source-clean
	$(call source-clean)
else
	$(call def-source-clean)
endif
