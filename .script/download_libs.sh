#!/bin/bash

SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd -P )"
SCRIPT_DIRNAME="$(basename "${SCRIPT_PATH}")"

LIBPATH=${SCRIPT_PATH}/../app/native-dist/libs

HOST="$(uname -s)"

case "${HOST}" in
    "Darwin")
        STRIP_ARG="--strip-components=1"
        ;;
    "Linux")
        STRIP_ARG="--strip-components 1"
        ;;
    *)
        echo "Error: Unsupported platform"
        exit 1;;
esac

cd /tmp

if [ $# -ge 1 ] ; then
    curl https://github.com/elastos/Elastos.NET.Carrier.Native.SDK/releases/tag/internal-test | grep -E "/elastos.*android.*gz|/elastos.*linux.*gz" -o >carrier_libs.txt
else
    curl https://github.com/elastos/Elastos.NET.Carrier.Native.SDK/releases/tag/internal-test | grep "/elastos.*android.*gz" -o >carrier_libs.txt
fi

sed -i 's/^/https:\/\/github.com/g' carrier_libs.txt
cat carrier_libs.txt
wget -i carrier_libs.txt

cd $LIBPATH
mkdir -p x86 && mkdir -p x86_64 && mkdir -p arm64-v8a && mkdir -p armeabi-v7a

tar $STRIP_ARG -zxf /tmp/*-android_x86-*.tar.gz -C x86/ lib
tar $STRIP_ARG -zxf /tmp/*-android_x86_64-*.tar.gz -C x86_64/ lib
tar $STRIP_ARG -zxf /tmp/*-android_arm64-*.tar.gz -C arm64-v8a/ lib
tar $STRIP_ARG -zxf /tmp/*-android_armv7a-*.tar.gz -C armeabi-v7a/ lib

if [ $# -ge 1 ] ; then
    cd /tmp && mkdir -p carrier && cd carrier
    ls /tmp/*-linux_x64* | xargs -n1 tar -zxf
fi

#check
