# Linux-x86_64

include environ/common.mk

LIB_SUFFIX= .so

SYSROOT_DIR   = "Dummy"
HOST_COMPILER = "Dummy"

CFLAGS   = -fPIC 
CFLAGS   += $(shell scripts/build.sh configFlags $(BUILD))

export CFLAGS

