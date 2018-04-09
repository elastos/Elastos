# Android-armv7

include environ/common.mk

ARM_ARCH=armeabi-v7a
APP_ABI=armeabi-v7a
LIB_SUFFIX= .so

HOST_COMPILER = arm-linux-androideabi
TOOLCHAIN_DIR = $(BUILD_DIR)/.android-toolchain/$(ARCH)
SYSROOT_DIR   = $(TOOLCHAIN_DIR)/sysroot
ANDROID_DEV   = $(SYSROOT_DIR)/usr
STDCPP_DIR    = $(shell ./scripts/android.sh "stdcpp_dir")

CFLAGS    = -Ofast -mthumb -marm -march=armv7-a
CFLAGS   += -mfloat-abi=softfp -mfpu=vfpv3-d16 
CFLAGS   += --sysroot=$(SYSROOT_DIR) -I$(SYSROOT_DIR)/usr/include
CFLAGS   += -Wall -fPIC -D__arm__ -D__ANDROID__
CFLAGS   += -DPJ_AUTOCONF
CFLAGS   += -I${STDCPP_DIR}/include -I${STDCPP_DIR}/libs/$(ARM_ARCH)/include
CFLAGS   += $(shell scripts/build.sh configFlags $(BUILD))

CPPFLAGS  = $(CFLAGS) -fexceptions

LIBS      = -L$(SYSROOT_DIR)/usr/lib -lc -lgcc
LIBS     += -L${STDCPP_DIR}/libs/$(ARM_ARCH) -lgnustl_static

LDFLAGS  += --sysroot=$(SYSROOT_DIR) -L$(SYSROOT_DIR)/usr/lib
LDFLAGS  += -L${STDCPP_DIR}/libs/$(ARM_ARCH)

ARFLAGS   =

CC        = $(TOOLCHAIN_DIR)/bin/clang
CXX       = $(TOOLCHAIN_DIR)/bin/clang++
AR        = $(TOOLCHAIN_DIR)/bin/llvm-ar
RANLIB    = $(TOOLCHAIN_DIR)/bin/arm-linux-androideabi-ranlib
LDSHARED  = $(TOOLCHAIN_DIR)/bin/clang
CPP       = $(TOOLCHAIN_DIR)/bin/clang -E
STRIP     = $(TOOLCHAIN_DIR)/bin/arm-linux-android-strip

export APP_ABI

export ANDROID_SYSROOT=$(SYSROOT_DIR)
export ANDROID_DEV
export CC
export CXX
export AR
export RANLIB
export LDSHARED
export CPP
export STRIP

export CFLAGS
export CPPFLAGS
export LIBS
export LDFLAGS
export ARFLAGS

