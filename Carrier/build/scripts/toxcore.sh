#!/bin/sh

#argv1: HOST
#argv2: ARCH
#argv3: HOST_COMPILER

CFLAGS="${CFLAGS} -fvisibility=hidden -DELASTOS_BUILD"

getConfigureCmd() {
    case $1,$2 in
        "Darwin",*)
            CFLAGS="${CFLAGS} -Wno-logical-op-parentheses"
            CFLAGS="${CFLAGS} -Wno-tautological-constant-out-of-range-compare"
            _TOOL="./configure"
            ;;
        "Linux","x86_64"| "Raspbian","armv7l")
            _TOOL="./configure --enable-epoll=yes"
            ;;
        "Android",*)
            _TOOL="./configure --host=$3 --enable-epoll=yes --disable-rt"
            ;;
        "iOS","x86")
            _TOOL="./configure --host=\"i386-apple-darwin_ios\""
            ;;
        "iOS","x86_64")
            _TOOL="./configure --host=\"x86_64-apple-darwin_ios\""
            ;;
        "iOS","arm" | "iOS","arm64")
            _TOOL="./configure --host=${3/D/d}"
            ;;
        *,*)
            echo "Unsupported platform $1:$2"
            exit 1;;
    esac

    echo "CFLAGS=\"${CFLAGS}\" ${_TOOL}"
}

if [ x"$1" = x"command" ]; then
    getConfigureCmd $2 $3 $4
    exit 0
fi

exit 0

