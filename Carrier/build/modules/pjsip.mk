include environ/$(HOST)-$(ARCH).mk

PACKAGE_NAME   = pjproject-2.5.5.tar.bz2
PACKAGE_URL    = http://www.pjsip.org/release/2.5.5/$(PACKAGE_NAME)
SRC_DIR        = $(DEPS_DIR)/pjproject-2.5.5

CONFIG_COMMAND = $(shell scripts/pjsip.sh "command" $(HOST) $(ARCH))
LIBRARY_SUFFIX = $(shell scripts/pjsip.sh "suffix"  $(HOST) $(ARCH))

ifeq (Android,$(HOST))
CFLAGS := $(CFLAGS) -DJNI_OnLoad=PJ_JNI_OnLoad
export CFLAGS
endif

CONFIG_OPTIONS = --prefix=$(DIST_DIR) \
        --disable-ssl \
        --disable-shared \
        --disable-resample \
        --disable-sound \
        --disable-oss \
        --disable-video \
        --disable-ext-sound  \
        --disable-speex-aec \
        --disable-g711-codec \
        --disable-l16-codec \
        --disable-gsm-codec \
        --disable-g722-codec \
        --disable-g7221-codec \
        --disable-speex-codec \
        --disable-ilbc-codec \
        --disable-libsamplerate \
        --disable-resample-dll \
        --disable-sdl \
        --disable-ffmpeg \
        --disable-v4l2 \
        --disable-openh264 \
        --disable-webrtc \
        --disable-ipp \
        --disable-opencore-amr \
        --disable-silk \
        --disable-opus \
        --disable-libyuv

define configure
    ./patch/pjsip.sh $(SRC_DIR) $(HOST) $(ARCH)
    cd $(SRC_DIR) && $(CONFIG_COMMAND) $(CONFIG_OPTIONS)
    cd $(SRC_DIR) && make dep
endef

define install
    cd $(SRC_DIR) && make install
    for m in pj pjlib-util pjnath pjmedia; do \
        cd $(DIST_DIR)/lib && mv lib$$m$(LIBRARY_SUFFIX).a lib$$m.a; \
    done
endef

define uninstall
    cd $(SRC_DIR) && PREFIX=$(DIST_DIR) make uninstall
    for m in pj pjlib-util pjnath; do \
        rm -f $(DIST_DIR)/lib/lib$$m.a; \
    done
endef

include modules/rules.mk

