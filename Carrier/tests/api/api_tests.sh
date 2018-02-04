#!/bin/sh

if [ $# != 0 ]; then
    echo "Error: Invalid command syntax."
    echo "Usage: api-tests.sh"
    echo ""
    exit 1
fi

HOST=$(uname -s)
ARCH=$(uname -m)
BUILD=debug

case "${HOST}" in
    "Darwin")
        DSO_ENV=DYLD_LIBRARY_PATH
        ;;
    "Linux")
        DSO_ENV=LD_LIBRARY_PATH
        ;;
    *)
        echo "Error: Unsupported platform"
        exit 1;;
esac

DYLIB_DIR=${PWD}/../../build/_dist/${HOST}-${ARCH}/${BUILD}/lib
if [ ! -d ${DYLIB_DIR} ]; then
    echo "Error: Library path ${DYLIB_DIR} not avialable"
    exit 1
fi

export ${DSO_ENV}=${DYLIB_DIR}

if [ ! -d .robot ]; then
    mkdir -p .robot
fi

if [ ! -d .tests ]; then
    mkdir -p .tests
fi

if [ ! -e api_tests ]; then
    echo "Error: api_tests not available."
    exit 1 
fi

./api_tests tests.conf $*

exit 0

