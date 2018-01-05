NEED_DEPS = $(if $(TARGET), \
                 $(filter $(filter $(TARGET), make install dist source-clean), $(TARGET)), \
                 "install" \
             )

.PHONY: all carrier
all: carrier 

$(MODULE):
	make -f modules/$(MODULE).mk $(TARGET)

%:
	make -f modules/$@.mk $(TARGET)

ifneq (, $(strip $(NEED_DEPS)))
toxcore: libsodium

ifneq (, $(findstring $(HOST), iOS Android))
carrier: libsodium toxcore flatcc pjsip
else
carrier: libsodium toxcore flatcc pjsip confuse CUnit
endif

endif

help:
	@echo "Usage: MODULE=[module] TARGET=[target] HOST=[platform] ARCH=[machine] BUILD=[build] make [module]"
	@echo "module options:"
	@echo "    confuse"
	@echo "    CUnit"
	@echo "    pjsip"
	@echo "    libsodium"
	@echo "    toxcore"
	@echo "    carrier"
	@echo ""
	@echo "platform options:"
	@echo "    Linux(default)"
	@echo "    Darwin"
	@echo "    iOS"
	@echo "    Android"
	@echo ""
	@echo "machine options:"
	@echo "    x86_64(default)"
	@echo "    arm64"
	@echo "    armv7l"
	@echo ""
	@echo "build options:"
	@echo "    debug(default)"
	@echo "    release"
	@echo ""
	@echo "target options:"
	@echo "    source"
	@echo "    config"
	@echo "    make"
	@echo "    install"
	@echo "    dist"
	@echo "    source-clean"
	@echo "    config-clean"
	@echo "    clean"
	@echo "    uninstall"
	@echo ""
	@echo "Help option:"
	@echo "    help"
	@echo ""
