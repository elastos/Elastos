#!/bin/sh

DEVROOT=$(xcode-select -print-path)

set -eo pipefail

getToolChainPath() {
    echo "${DEVROOT}/Toolchains/XcodeDefault.xctoolchain"
}

if [ x"$1" = x"toolchain" ]; then
     getToolChainPath
     exit 0
fi

getCrossTopPath() {
     __TARGET=
     case x"${1:0:3}" in
        x"arm")
            __TARGET="iPhoneOS"
            ;;
        x"x86")
            __TARGET="iPhoneSimulator"
            ;;
        *)
            return
     esac

     echo "${DEVROOT}/Platforms/${__TARGET}.platform/Developer"
}

if [ x"$1" = x"crosstop" ]; then
    getCrossTopPath $2
    exit 0
fi

getCrossSdkName() {
     __TMP=".__tmpsdk"
     __PATH=$(getCrossTopPath $1)

     for sdk in $(ls "${__PATH}/SDKs/"); do
         echo $(echo ${sdk} | sed 's/\(.sdk\)//') >> ${__TMP}
     done
     
     echo $(cat ${__TMP} | sort | tail -1)".sdk"
     rm -f ${__TMP}
}

if [ x"$1" = x"crosssdk" ]; then
    getCrossSdkName $2
    exit 0
fi

getSysRootPath() {
    echo $(getCrossTopPath $1)"/SDKs/"$(getCrossSdkName $1)
}

if [ x"$1" = x"sysroot" ]; then
    getSysRootPath $2
    exit 0
fi

exit 0

