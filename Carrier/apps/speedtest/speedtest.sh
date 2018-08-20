#!/bin/bash

SCRIPT_PATH="$( cd "$(dirname "$0")" ; pwd -P )"
SCRIPT_DIRNAME="$(basename "${SCRIPT_PATH}")"

# Running in installation or dist directory
LDPATH="$(dirname "${SCRIPT_PATH}")/lib"

if [ ! -e ${SCRIPT_PATH}/elashell ]; then
    echo "Error: elaspeedtest program not available."
    exit 1
fi

HOST="$(uname -s)"

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

export ${DSO_ENV}=${LDPATH}

cd ${SCRIPT_PATH} && ./elaspeedtest -c ${SCRIPT_PATH}/elaspeedtest.conf $*