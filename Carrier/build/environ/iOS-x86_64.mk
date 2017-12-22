# iOS-x86_64

include environ/common.mk

LIB_SUFFIX= .dylib

TOOLCHAIN_DIR = $(shell ./scripts/ios.sh "toolchain" $(ARCH))
SYSROOT_DIR   = $(shell ./scripts/ios.sh "sysroot" $(ARCH))
CROSSTOP_DIR  = $(shell ./scripts/ios.sh "crosstop" $(ARCH))
CROSSSDK_NAME = $(shell ./scripts/ios.sh "crosssdk" $(ARCH))
HOST_COMPILER = $(shell uname -m)-apple-$(shell uname -s)$(shell uname -r)

CC        = $(TOOLCHAIN_DIR)/usr/bin/clang -arch $(ARCH)
AR        = $(TOOLCHAIN_DIR)/usr/bin/ar
RANLIB    = echo ranlib

CFLAGS    = -mios-simulator-version-min=7.0
CFLAGS   += --sysroot=$(SYSROOT_DIR) -I$(SYSROOT_DIR)/usr/include
CFLAGS   += -Wall -fPIC -D__x86_64__ -D__APPLE__
CFLAGS   += -Wno-implicit-function-declaration -Wno-nullability-completeness
CFLAGS   += -DPJ_AUTOCONF
CFLAGS   += $(shell scripts/build.sh configFlags $(BUILD))

LDFLAGS   = -mios-simulator-version-min=7.0 
LDFLAGS  += --sysroot=$(SYSROOT_DIR) -L$(SYSROOT_DIR)/usr/lib

ARFLAGS   = 
CPP       = $(CC) -E --sysroot=$(SYSROOT_DIR) 
CPP      += -I$(SYSROOT_DIR)/usr/include
CPP      += -L$(SYSROOT_DIR)/usr/lib

export SYSROOT_DIR
export CC
export CFLAGS
export LDFLAGS
export AR
export ARFLAGS
export RANLIB
export CPP
export DEVPATH  =$(CROSSTOP_DIR)
export IPHONESDK=$(CROSSTOP_DIR)/SDKs/$(CROSSSDK_NAME)

