#!/bin/sh

TARGET_OS="Android"
TARGET_ARCH="arm"
TARGET_MAKE="install"
TARGET_BUILD="debug"
TARGET_MODULE="carrier"

if [ x"$(uname -s)" != x"Linux" ] && [ x"$(uname -s)" != x"Darwin" ]; then
    echo "Error: $0 should run on Linux/Darwin with Android-NDK"
    exit 1
fi

if [ -z "${ANDROID_NDK_HOME}" ]; then
    echo "Error: ANDROID_NDK_HOME environment not set"
    exit 1
fi

set -e

TARGET_TOOLCHAIN="arm-linux-androideabi-4.9"
TARGET_API="21"

while [ x"$1" != x ]; do
case "$1" in
    "arm" | "arm64" | "x86" | "x86_64" | "mips")
        TARGET_ARCH=$1
        shift;;

    "mips64")
        echo "Error: arch $1 not support yet"
        exit 1;;

    "libsodium" | "toxcore" | "flatcc" | "pjsip" | "carrier")
        TARGET_MODULE=$1
        shift;;

    "source" | "config" | "make" | "install" | \
    "source-clean" | "config-clean" | "clean" | "uninstall" | "dist")
        TARGET_MAKE="$1"
        shift;;

    "debug" | "release")
        TARGET_BUILD="$1"
        shift;;

    *)
        echo "Usage: $0 [ module ] [ arch ] [ target ] [ build ]"
        echo "module options(default carrier):"
        echo "    libsodium    |  toxcore     | flatcc   | pjsip   | carrier"
        echo ""
        echo "Default arch (arm(v7a):"
        echo "    arm(v7a)     | arm64(aarch64)"
        echo "    x86(i386)    | x86_64"
        echo "    mips(el)"
        echo ""
        echo "target options(default install):"
        echo "    source       | config       | make     | install"
        echo "    source-clean | config-clean | clean    | uninstall"
        echo "    dist"
        echo ""
        echo "build options(default:debug):"
        echo "    debug        | release"
        echo ""
        exit 0
        ;;
esac
done

TOOLCHAIN_DIR="$(pwd)/.android-toolchain/${TARGET_ARCH}"

if [ ! -d ${TOOLCHAIN_DIR} ]; then
    MAKE_SCRIPT="${ANDROID_NDK_HOME}/build/tools/make_standalone_toolchain.py"
    if [ ! -x ${MAKE_SCRIPT} ]; then
        echo "Error: Script make_standalone_toolchain.py not found or run-forbidden"
        exit 1
    fi

    ${MAKE_SCRIPT} --arch="${TARGET_ARCH}" \
            --verbose \
            --api="${TARGET_API}" \
            --stl="gnustl" \
            --install-dir="${TOOLCHAIN_DIR}"

fi

MODULE=${TARGET_MODULE} \
    TARGET=${TARGET_MAKE} \
    HOST=${TARGET_OS} \
    ARCH=${TARGET_ARCH} \
    BUILD=${TARGET_BUILD} make ${TARGET_MODULE}

