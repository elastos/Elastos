#!/bin/sh

PACKAGE_URL=$1
PACKAGE=$2
REPO_DIR=$3
DEPS_DIR=$4

if [ $(echo ${PACKAGE_URL} | grep -c 'github.com') -eq 1 ]; then
    _REDIRECT="-L"
fi

if [ ! -e ${REPO_DIR}/${PACKAGE} ]; then
    mkdir -p ${REPO_DIR}
    echo "curl ${_REDIRECT} ${PACKAGE_URL} -o ${REPO_DIR}/${PACKAGE}"
    curl ${_REDIRECT} ${PACKAGE_URL} -o ${REPO_DIR}/${PACKAGE}
fi

if [ ! -e ${DEPS_DIR} ]; then
   mkdir -p ${DEPS_DIR}
fi

if [ $(echo ${PACKAGE} | grep -c 'zip\>') -eq 1 ]; then
    echo "unzip ${REPO_DIR}/${PACKAGE} -d -o ${DEPS_DIR}"
    unzip -o ${REPO_DIR}/${PACKAGE} -d ${DEPS_DIR}
    exit 0
fi

if [ $(echo ${PACKAGE} | grep -c 'tar.gz\>') -eq 1 ]; then
    echo "tar -xzf ${REPO_DIR}/${PACKAGE} -C ${DEPS_DIR}"
    tar -xzvf ${REPO_DIR}/${PACKAGE} -C ${DEPS_DIR}
    exit 0
fi

if [ $(echo ${PACKAGE} | grep -c 'tar.bz2\>') -eq 1 ]; then
    echo "tar -xzf ${REPO_DIR}/${PACKAGE} -C ${DEPS_DIR}"
    tar -xjvf ${REPO_DIR}/${PACKAGE} -C ${DEPS_DIR}
    exit 0
fi

exit 0

