# Linux-x86_64

include environ/common.mk

LIB_SUFFIX= .so

SYSROOT_DIR   = "Dummy"
HOST_COMPILER = "Dummy"

CFLAGS   = -fPIC 
CFLAGS   += $(shell scripts/build.sh configFlags $(BUILD))

export CFLAGS

ifeq ($(TOOLCHAIN),clang)
CC        = clang
CXX       = clang++
AR        = llvm-ar
RANLIB    = llvm-ranlib
LDSHARED  = clang
CPP       = clang -E

export CC
export CXX
export AR
export RANLIB
export LDSHARED
export CPP
endif
