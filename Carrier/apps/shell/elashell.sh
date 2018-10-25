#!/bin/bash

DATA_DIR=~/.elashell

SCRIPT_PATH="$( cd "$(dirname "$0")" ; pwd -P )"
SCRIPT_DIRNAME="$(basename "${SCRIPT_PATH}")"
CONFIG_FILE="$(dirname "${SCRIPT_PATH}")/etc/carrier/elashell.conf"

# Running in installation or dist directory
LDPATH="$(dirname "${SCRIPT_PATH}")/lib"

if [ ! -e ${SCRIPT_PATH}/elashell ]; then
    echo "Error: elashell program not available."
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

${SCRIPT_PATH}/elashell -c ${CONFIG_FILE} $*
