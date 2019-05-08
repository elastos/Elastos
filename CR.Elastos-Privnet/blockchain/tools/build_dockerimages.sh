#!/bin/bash

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

CURRENT_DIR=$(pwd)

function build_binary_and_docker {
    BRANCH="${1}"
    REPO="${GOPATH}/src/github.com/elastos/${2}"
    WORKDIR="${3}"
    BINARY="${4}"
    DOCKERIMAGE="${5}"

    cd $REPO
    git checkout $BRANCH
    git pull
    mkdir -p $TMPDIR/$WORKDIR/$BINARY
    cp $CURRENT_DIR/$WORKDIR/* $TMPDIR/$WORKDIR/
    cp -r * $TMPDIR/$WORKDIR/$BINARY/
    docker build -t $DOCKERIMAGE -f $TMPDIR/$WORKDIR/Dockerfile $TMPDIR/$WORKDIR/
    cd $CURRENT_DIR
}

build_binary_and_docker "release_v0.3.2" "Elastos.ELA" "ela-mainchain" "ela" \
    "cyberrepublic/elastos-mainchain-node:latest"

build_binary_and_docker "release_v0.1.1" "Elastos.ELA.Arbiter" "ela-arbitrator" "arbiter" \
    "cyberrepublic/elastos-arbitrator-node:latest"

build_binary_and_docker "release_v0.1.2" "Elastos.ELA.SideChain.ID" "ela-sidechain/did" "did" \
    "cyberrepublic/elastos-sidechain-did-node:latest"

build_binary_and_docker "release_v0.1.2" "Elastos.ELA.SideChain.Token" "ela-sidechain/token" "token" \
    "cyberrepublic/elastos-sidechain-token-node:latest"

build_binary_and_docker "master" "Elastos.ORG.Wallet.Service" "restful-services/wallet-service" "service" \
    "cyberrepublic/elastos-wallet-service:latest"

build_binary_and_docker "master" "Elastos.ORG.DID.Service" "restful-services/did-service" "service" \
    "cyberrepublic/elastos-did-service:latest"

build_binary_and_docker "master" "Elastos.ORG.API.Misc" "restful-services/api-misc" "misc" \
    "cyberrepublic/elastos-api-misc-service:latest"

cd $CURRENT_DIR