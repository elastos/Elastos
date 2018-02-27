#!/bin/sh

HOST="$(uname -s)"
ARCH="$(uname -m)"

BUILD=debug

MODE=$1

case ${MODE} in
    "client" | "server")
        ;;
    *)
        echo "Error: Invalid command syntax."
        echo "USAGE: ./elapfd.sh client | server"
        echo ""
        ;;
esac

case "${HOST}" in
    "Darwin")
        DSO_ENV=DYLD_LIBRARY_PATH
        ;;
    "Linux")
        DSO_ENV=LD_LIBRARY_PATH
        ;;
    *)
        echo "Error: Unsupported platform ${HOST}"
        exit 1;;
esac

export ${DSO_ENV}=${PWD}/../../build/_dist/${HOST}-${ARCH}/${BUILD}/lib

CUR_DIR=${PWD}
RUN_DIR=${PWD}
DATA_DIR=.${MODE}-data

if [ ! -e ${RUN_DIR}/elapfd ]; then
    echo "Error: elapfd not available."
    exit 1
fi

if [ ! -d ${DATA_DIR} ]; then
    mkdir -p ${DATA_DIR}
fi

cd ${RUN_DIR} && ./elapfd -c ${CUR_DIR}/${MODE}.conf $*

exit 0

