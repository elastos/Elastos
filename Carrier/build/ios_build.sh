#!/bin/sh
TARGET_OS="iOS"
TARGET_ARCH="$(uname -m)"
TARGET_MAKE="install"
TARGET_BUILD="debug"
TARGET_MODULE="carrier"

set -eo pipefail

if [ x"$(uname -s)" != x"Darwin" ] && [ x"$(uname -s)" != x"iOS" ]; then
    echo "Error: $0 should run on Darwin OS"
    exit 1
fi

while [ x"$1" != x ]; do
case "$1" in
    "x86" | "x86_64" | "arm64" | "arm")
        TARGET_ARCH="$1"
        shift;;

    "pjsip" | "libsodium" | "toxcore" | "carrier")
        TARGET_MODULE="$1"
        shift;;

    "source" | "config" | "make" | "install" | \
    "source-clean" | "config-clean" | "clean" | "uninstall" | "dist")
        TARGET_MAKE="$1"
        shift;;

    "debug" | "release")
        TARGET_BUILD="$1"
        shift;;

    *)
        echo "Usage: $0 [ module ] [ arch ] [ target ] [  build ]"
        echo "module options(default carrier):"
        echo "    pjsip        | libsodium    | toxcore | carrier"
        echo ""
        echo "arch options(default $(uname -m)):"
        echo "    x86          | x86_64       | arm64   | arm"
        echo ""
        echo "target options(default install):"
        echo "    source       | config       | make    | install"
        echo "    source-clean | config-clean | clean   | uninstall"
        echo "    dist"
        echo ""
        echo "build options(default:debug):"
        echo "    debug        | release"
        echo ""
        exit 0
        ;;
esac
done

MODULE=${TARGET_MODULE} \
    TARGET=${TARGET_MAKE} \
    HOST=${TARGET_OS} \
    ARCH=${TARGET_ARCH} \
    BUILD=${TARGET_BUILD} make ${TARGET_MODULE}

