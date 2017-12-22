include environ/$(HOST)-$(ARCH).mk

SRC_DIR  = $(ROOT_DIR)/src
APPS_DIR = $(ROOT_DIR)/apps
TEST_DIR = $(ROOT_DIR)/tests

define source-fetch
    @echo "Dummy source fetch ..."
endef

define configure
    @echo "Dummy configure ..."
endef

define compile
    for m in $(SRC_DIR) $(APPS_DIR) $(TEST_DIR); do \
        cd $$m && PREFIX=$(DIST_DIR) make; \
    done
endef

define install
    for m in $(SRC_DIR) $(APPS_DIR) $(TEST_DIR); do \
        cd $$m && PREFIX=$(DIST_DIR) make install; \
    done
    ./scripts/carrier.sh "postInstall" $(HOST) $(DIST_DIR)
endef

define dev-dist
    ./scripts/carrier.sh "packDevDist" $(HOST) $(ARCH) $(BUILD) $(DIST_DIR)
endef

define uninstall
    @echo "Dummy uninstall ..."
endef

define make-clean
    for m in $(SRC_DIR) $(APPS_DIR) $(TEST_DIR); do \
        cd $$m && PREFIX=$(DIST_DIR) make clean; \
    done
endef

define config-clean
    $(call make-clean)
endef

define source-clean
    $(call make-clean)
endef

source:
	$(call source-fetch)

config: source
	$(call configure)

make: config
	$(call compile) 

install: make
	mkdir -p $(DIST_DIR)
	$(call install)

dist: install
	$(call dev-dist)

clean:
	$(call make-clean)

config-clean:
	$(call config-clean)

source-clean:
	$(call source-clean)
