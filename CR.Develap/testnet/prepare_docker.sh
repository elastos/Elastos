#!/bin/bash

DOCKER_DIR="/data/docker/volumes/development-services/testnet"

mkdir -p ${DOCKER_DIR}/ela ${DOCKER_DIR}/did ${DOCKER_DIR}/token
cp -r mainchain/* ${DOCKER_DIR}/ela
cp -r did/* ${DOCKER_DIR}/did
cp -r token/* ${DOCKER_DIR}/token