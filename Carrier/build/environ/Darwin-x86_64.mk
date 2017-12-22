# Darwin-x86_64

include environ/common.mk

LIB_SUFFIX= .dylib

SYSROOT_DIR   = "Dummy"
HOST_COMPILER = "Dummy"

CFLAGS    = $(shell scripts/build.sh configFlags $(BUILD))

export CFLAGS

