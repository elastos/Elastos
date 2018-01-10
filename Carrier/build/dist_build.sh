#!/bin/sh

set -e

RUN_HOST=$(uname -s)

if [ ${RUN_HOST} != "Linux" ] && [ ${RUN_HOST} != "Darwin" ]; then
    echo "Error: This script should run on Linux/Darwin"
    exit 1
fi

darwin_build() {
    for arch in "x86_64"; do
        for build in $*; do
            ./darwin_build.sh ${arch} ${build} clean
            ./darwin_build.sh ${arch} ${build} dist
        done
    done
}

lipo_process() {
    for build in $* ; do
        LIPO_PATH=_dist/lipo/${build}

        rm -rf ${LIPO_PATH} &&
            mkdir -p ${LIPO_PATH}

        LIBS="sodium toxcore flatcc flatccrt pj pjlib-util pjnath pjmedia \
              elacommon elacarrier elasession"

        for lib in ${LIBS}; do 
            LPLIBS=
            for arch in "arm64" "x86_64"; do
                LPLIBS="${LPLIBS} _dist/iOS-${arch}/${build}/lib/lib${lib}.a"
            done
            lipo ${LPLIBS} -create -output ${LIPO_PATH}/lib${lib}.a 
        done 
   done
}

ios_build() {
    for arch in "arm64" "x86_64"; do
        for build in $*; do
            ./ios_build.sh ${arch} ${build} clean
            ./ios_build.sh ${arch} ${build} dist 
        done
    done

    lipo_process $*
}

linux_build() {
    for arch in "x86_64" "armv7l"; do
        for build in $*; do
            ./linux_build.sh ${arch} ${build} clean
            ./linux_build.sh ${arch} ${build} dist
        done
    done
}

android_build() {
    for arch in "arm" "arm64" "x86" "x86_64"; do
        for build in $*; do
            ./android_build.sh ${arch} ${build} clean
            ./android_build.sh ${arch} ${build} dist
        done
    done
}

TARGETS="all"
BUILDS="both"

while [ x"$1" != x ]; do
case "$1" in
    "darwin" | "ios" | "android" | "linux")
        TARGETS="$1"
        shift;;

    "all")
        TARGETS="$1"
        shift;;

    "debug" | "release")
        BUILDS="$1"
        shift;;

    "both")
        BUILDS="$1"
        shift;;

    *)
        echo "Usage: $0 [ target ] [ build ]"
        echo "target options (default all):"
        echo "    darwin  |  ios  |  android |  linux"
        echo "    all"
        echo ""
        echo "build options (default: both):"
        echo "    debug   |  release"
        echo "    both"
        echo ""
        exit 0;;
esac
done

case "${RUN_HOST}" in
    "Darwin")
        case ${TARGETS} in
            "linux")
                echo "Error: Unsupported for build ${TARGETS} distribution on ${RUN_HOST}"
                exit 1;;
            "all")
                TARGETS="darwin ios android"
                ;;
        esac
        ;;

    "Linux")
        case ${TARGETS} in
            "darwin" | "ios")
                echo "Error: Unsupported for build ${TARGETS} distribution on ${RUN_HOST}"
                exit 1;;
            "all")
                TARGETS="android linux"
                ;;
        esac
        ;;
esac

case "${BUILDS}" in
     "debug" | "release")
         ;;
     "both")
         BUILDS="debug release"
         ;;
esac

for target in ${TARGETS}; do
    ${target}_build ${BUILDS}
done

exit 0

