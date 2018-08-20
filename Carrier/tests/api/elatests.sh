#!/bin/bash

if [ $# != 0 ]; then
    echo "Error: Invalid command syntax."
    echo "Usage: elatests.sh"
    echo ""
    exit 1
fi

SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd -P )"
SCRIPT_DIRNAME="$(basename "${SCRIPT_PATH}")"

# Running in the installation or dist directory
LDPATH="$(dirname "${SCRIPT_PATH}")/lib"
CONFIG_FILE="$(dirname "${SCRIPT_PATH}")/etc/carrier/tests.conf"

if [ ! -e ${CONFIG_FILE} ]; then
    echo "Error: Elastos Carrier api tests config file not available"
    exit 1
fi

if [ ! -e ${SCRIPT_PATH}/elatests ]; then
    echo "Error: Elastos Carrier api tests program not available."
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

if [ "$1" != "" ]; then
    ${SCRIPT_PATH}/elatests $*
else
    ${SCRIPT_PATH}/elatests ${CONFIG_FILE} $*
fi
