#!/bin/bash

CURRENT_DIR=$(pwd)

function setup_node() {
    TYPE_DIR=${1}
    SUB_DIR=${2}

    cd ela-${TYPE_DIR}/${SUB_DIR}
    for dir in $(ls -d */)
    do
        rm -rf ${dir}/backup ${dir}/elastos*
        absolute_path_dir="`pwd`/${dir}"
        container_name=$(echo $dir | sed 's:/*$::')
        if [ "${TYPE_DIR}" == "mainchain" ]
        then
            docker cp ela-${container_name}:/root/ela/elastos ${absolute_path_dir}/backup
            docker cp ela-${container_name}:/root/ela/elastos ${absolute_path_dir}
        elif [ "${TYPE_DIR}" == "arbitrator" ]
        then
            docker cp ela-${container_name}:/root/arbiter/elastos_arbiter ${absolute_path_dir}/backup
            docker cp ela-${container_name}:/root/arbiter/elastos_arbiter ${absolute_path_dir}
        else
            docker cp ela-sidechain-${container_name}:/root/${SUB_DIR}/elastos_${SUB_DIR} ${absolute_path_dir}/backup
            docker cp ela-sidechain-${container_name}:/root/${SUB_DIR}/elastos_${SUB_DIR} ${absolute_path_dir}
        fi
    done
    cd ${CURRENT_DIR}
}

setup_node "mainchain" "node_crc"
setup_node "mainchain" "node_dpos"
setup_node "mainchain" "node_normal"

setup_node "arbitrator" "node_crc"
setup_node "arbitrator" "node_origin"

setup_node "sidechain" "did"

setup_node "sidechain" "token"