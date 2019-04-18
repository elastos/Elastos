#!/bin/bash

SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd -P )"
SCRIPT_DIRNAME="$(basename "${SCRIPT_PATH}")"

LIBPATH=${SCRIPT_PATH}/../NativeDistributions
LIBPATTERN="/elastos.*ios_arm64.*gz"
LIBDIR="-iphoneos"
LIBPACKAGE="ios_arm64"

if [ $1 != "arm64" ] ; then
    LIBPATTERN="/elastos.*ios_x64.*gz"
    LIBDIR="-iphonesimulator"
    LIBPACKAGE="ios_x64"
fi

packageUrl=`curl https://github.com/elastos/Elastos.NET.Carrier.Native.SDK/releases/tag/internal-test | grep -e $LIBPATTERN -o`
libPackageName=${packageUrl##*/}

cd /tmp
echo "https://github.com"${packageUrl} >carrier_libs.txt

#remove old package
rm ${libPackageName}

wget -i carrier_libs.txt

cd ${LIBPATH}
mkdir lib

cd lib
mkdir -- ${LIBDIR}
tar --strip-components=1 -zxf /tmp/${libPackageName} -C ${LIBDIR}/ lib
