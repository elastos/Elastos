#!/bin/sh

getConfigureCmd() {
    _ENVS="CFLAGS=\"-fPIC\" ARCH=\"-arch $2\""
    _EPOLL="--enable-epoll"
    case $1,$2 in
        "Linux","x86_64" | "Raspbian",*)
            _CMD="./configure"
            ;;
        "Linux","armv7l")
            _CMD="./configure --host=arm-linux-gnueabi"
            ;;
        "Darwin",*)
            _CMD="./configure"
            _EPOLL=""
            ;;
        "iOS","x86_64" | "iOS","arm64")
            _CMD="./configure-iphone"
            _EPOLL=""
            ;;
        "iOS","arm")
            _ENVS="CFLAGS=\"-fPIC\" ARCH=\"-arch armv7\""
            _CMD="./configure-iphone"
            _EPOLL=""
            ;;
        "iOS","x86")
            _ENVS="CFLAGS=\"-fPIC\" ARCH=\"-arch i386\""
            _CMD="./configure-iphone"
            _EPOLL=""
            ;;
        "Android","arm")
            _CMD="./configure --host=$2-linux-androideabi"
            ;;
        "Android","arm64")
            _CMD="./configure --host=aarch64-linux-android"
            ;;
        "Android","x86")
            _CMD="./configure --host=i686-linux-android"
            ;;
        "Android","x86_64")
            _CMD="./configure --host=x86_64-linux-android"
            ;;
        "Android","mips")
            _CMD="./configure --host=mipsel-linux-android"
            ;;
        "Android","mips64")
            _CMD="./configure --host=mips64el-linux-android"
            ;;
        *)
            echo "Error: Unsupported Host:Arch($1:$2) for pjsip configure"
            exit 1
            ;;
    esac

    echo "${_ENVS} ${_CMD} ${_EPOLL}"
}

if [ x"$1" = x"command" ]; then
    getConfigureCmd $2 $3
    exit 0
fi

getLibrarySuffix() {
    case $1,$2 in
        "Linux","x86_64")
            _SUFFIX="-$2-unknown-linux-gnu"
            ;;
        "Linux","armv7l")
            _SUFFIX="-arm-unknown-linux-gnueabi"
            ;;
        "Raspbian","armv7l")
            _GNUSPEC=$(gcc -dumpmachine | awk -F '-' '{print $3}')
            _SUFFIX="-$2-unknown-linux-${_GNUSPEC}"
            ;;
        "Darwin","x86_64")
            _GNUSPEC=$(gcc -dumpmachine)
            _SUFFIX="-${_GNUSPEC}"
            ;;
        "iOS","arm")
            _SUFFIX="-armv7-apple-darwin_ios"
            ;;
        "iOS","arm64")
            _SUFFIX="-arm64-apple-darwin_ios"
            ;;
        "iOS","x86")
            _SUFFIX="-i386-apple-darwin_ios"
            ;;
        "iOS","x86_64")
            _SUFFIX="-x86_64-apple-darwin_ios"
            ;;
        "Android","arm")
            _SUFFIX="-arm-unknown-linux-androideabi"
            ;;
        "Android","x86")
            _SUFFIX="-i686-pc-linux-android"
            ;;
        "Android","mips")
            _SUFFIX="-mipsel-unknown-linux-android"
            ;;
        "Android","x86_64")
            _SUFFIX="-x86_64-pc-linux-android"
            ;;
        "Android","arm64")
            _SUFFIX="-aarch64-unknown-linux-android"
            ;;
        *,*)
            echo "Error: Unspported platform($1:$2) for Android platform"
            exit 1;;
    esac

    echo ${_SUFFIX}
}

if [ x"$1" = x"suffix" ]; then
    getLibrarySuffix $2 $3
    exit 0
fi

