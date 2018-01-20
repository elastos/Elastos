#/bin/sh

HOST="$(uname -s)"
ARCH="$(uname -m)"

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

export ${DSO_ENV}=${PWD}/../../build/_dist/${HOST}-${ARCH}/${BUILD}/lib

RUN_DIR=${PWD}

if [ ! -e ${RUN_DIR}/elashell ]; then
    echo "Error: elashell not available."
    exit 1
fi

cd ${RUN_DIR} && ./elashell -c ${RUN_DIR}/elashell.conf $*

