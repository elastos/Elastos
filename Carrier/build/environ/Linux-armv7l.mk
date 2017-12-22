# Linux armv7l (raspberry specific environment script)
# toolschain package address: https://github.com/raspberrypi/tools

include environ/common.mk

LIB_SUFFIX= .so

TOOLCHAIN_DIR = $(RASPBERRY_TOOLCHAIN_HOME)/gcc-linaro-arm-linux-gnueabihf-raspbian-x64
SYSROOT_DIR   = $(TOOLCHAIN_DIR)/arm-linux-gnueabihf/libc
HOST_COMPILER = arm-linux-gnueabihf
TOOLCHAIN_PREFIX = arm-linux-gnueabihf-

CFLAGS   = -Ofast -mthumb -marm -march=armv7-a
CFLAGS  += -mfloat-abi=hard -mfpu=vfpv3-d16
CFLAGS  += --sysroot=$(SYSROOT_DIR) -I$(SYSROOT_DIR)/usr/include
CFLAGS  += -Wall -fPIC -D__arm__
CFLAGS  += -DPJ_AUTOCONF
CFLAGS  += $(shell scripts/build.sh configFlags $(BUILD))

CPPFLAGS = $(CFLAGS) -fexceptions
LIBS     = -L$(SYSROOT_DIR)/lib/arm-linux-gnueabihf -lc -lrt
LIBS    += -L$(TOOLCHAIN_DIR)/arm-linux-gnueabihf/lib  -lc -lgcc_s
LIBS    += -L$(TOOLCHAIN_DIR)/lib/gcc/arm-linux-gnueabihf/4.9.3

LDFLAGS  = -L$(SYSROOT_DIR)/usr/lib/arm-linux-gnueabihf

ARFLAGS  =

CC       = $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)gcc
CXX      = $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)g++
AR       = $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)ar
RANLIB   = $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)ranlib
LDSHARED = $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)gcc
CPP      = $(TOOLCHAIN_DIR)/bin/$(TOOLCHAIN_PREFIX)cpp

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

