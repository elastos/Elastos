# Raspbian armv7l

include environ/common.mk

LIB_SUFFIX= .so

SYSROOT_DIR   = "Dummy"
HOST_COMPILER = "Dummy"

CFLAGS   += $(shell scripts/build.sh configFlags $(BUILD))
CFLAGS   += -DPJ_AUTOCONF

export CFLAGS

