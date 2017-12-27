#!/bin/sh

TARGET_OS="Raspbian"
TARGET_ARCH="$(uname -m)"
TARGET_MAKE="install"
TARGET_BUILD="debug"
TARGET_MODULE="carrier"

set -e

if [ x"$(uname -s)" != x"Linux" ]; then
    echo "Error: $0 should run on Raspbian OS"
    exit 1
fi

while [ x"$1" != x ]; do
case "$1" in
    "libsodium" | "toxcore" | "flatcc" | "pjsip" | "carrier")
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
        echo "Usage: $0 [ module ] [ build ] [ target ]"
        echo "module options(default: carrier):"
        echo "    libsodium    | toxcore      | flatcc   | pjsip    | carrier"
        echo ""
        echo "target options(default install)"
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

MODULE=${TARGET_MODULE} \
    TARGET=${TARGET_MAKE} \
    HOST=${TARGET_OS} \
    ARCH=${TARGET_ARCH} \
    BUILD=${TARGET_BUILD} make ${TARGET_MODULE}

