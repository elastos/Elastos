#!/bin/bash

docker-compose stop 

CURRENT_DIR=$(pwd)
DOCKER_VOLUME=/data/volumes/elastos-privnet
sudo rm -rf ${DOCKER_VOLUME}
mkdir -p ${DOCKER_VOLUME}

function setup_node() {
    SRC_DIR=${1}

    cd ${SRC_DIR}
    for dir in $(ls -d */ | grep "-")
    do
        mkdir ${DOCKER_VOLUME}/${dir}/
        cp -r ${dir}/* ${DOCKER_VOLUME}/${dir}/
    done
    cd ${CURRENT_DIR}
}

setup_node "ela-mainchain/node_crc"
setup_node "ela-mainchain/node_dpos"
setup_node "ela-mainchain/node_normal"

setup_node "ela-arbitrator/node_crc"
setup_node "ela-arbitrator/node_origin"

setup_node "ela-sidechain/did"

setup_node "ela-sidechain/token"

setup_node "ela-sidechain/eth"

setup_node "restful-services"