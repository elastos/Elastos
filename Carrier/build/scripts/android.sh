#!/bin/sh

getStdCppDir() {
    _DIR=${ANDROID_NDK_HOME}/sources/cxx-stl/gnu-libstdc++
    echo $(ls -d ${_DIR}/[0-9]* | sort -gr | head -1)
}

if [ x"$1" = x"stdcpp_dir" ]; then
    getStdCppDir 
    exit 0
fi

exit 0

