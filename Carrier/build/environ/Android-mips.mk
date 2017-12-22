# Android-mipsel

include environ/common.mk

LIB_SUFFIX= .so

HOST_COMPILER = mipsel-linux-android
TOOLCHAIN_DIR = $(BUILD_DIR)/.android-toolchain/$(ARCH)
SYSROOT_DIR   = $(TOOLCHAIN_DIR)/sysroot
ANDROID_DEV   = $(SYSROOT_DIR)/usr
STDCPP_DIR    = $(shell ./scripts/android.sh "stdcpp_dir")

CFLAGS    = -Ofast
CFLAGS   += --sysroot=$(SYSROOT_DIR) -I$(SYSROOT_DIR)/usr/include
CFLAGS   += -Wall -fPIC -D__mips__ -D__ANDROID__
CFLAGS   += -DPJ_AUTOCONF
CFLAGS   += -DPJ_IS_LITTLE_ENDIAN=1 -DPJ_IS_BIG_ENDIAN=0
CFLAGS   += -I${STDCPP_DIR}/include -I${STDCPP_DIR}/libs/${ARCH}/include
CFLAGS   += $(shell scripts/build.sh configFlags $(BUILD))

CPPFLAGS  = $(CFLAGS) -fexceptions

LIBS      = -L$(SYSROOT_DIR)/usr/lib -lc -lgcc
LIBS     += -L${STDCPP_DIR}/libs/${ARCH} -lgnustl_static

LDFLAGS  += --sysroot=$(SYSROOT_DIR) -L$(SYSROOT_DIR)/usr/lib
LDFLAGS  += -L${STDCPP_DIR}/libs/${ARCH}

ARFLAGS   = 

CC        = $(TOOLCHAIN_DIR)/bin/mipsel-linux-android-gcc
CXX       = $(TOOLCHAIN_DIR)/bin/mipsel-linux-android-g++
AR        = $(TOOLCHAIN_DIR)/bin/mipsel-linux-android-ar
RANLIB    = $(TOOLCHAIN_DIR)/bin/mipsel-linux-android-ranlib
LDSHARED  = $(TOOLCHAIN_DIR)/bin/mipsel-linux-android-gcc
CPP       = $(TOOLCHAIN_DIR)/bin/mipsel-linux-android-cpp

export ANDROID_SYSROOT=$(SYSROOT_DIR)
export ANDROID_DEV
export CC
export CXX
export AR
export RANLIB
export LDSHARED
export CPP

export CFLAGS
export CPPFLAGS
export LIBS
export LDFLAGS
export ARFLAGS

