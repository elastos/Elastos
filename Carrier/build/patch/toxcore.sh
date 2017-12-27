#!/bin/sh

#argv1: SRC_DIR

SRC_DIR=$1
shift

echo "SRC_DIR: ${SRC_DIR}"

cd ${SRC_DIR}/.. &&
patch -s -p0 < "${SRC_DIR}/../../../../patch/toxcore.patch"
