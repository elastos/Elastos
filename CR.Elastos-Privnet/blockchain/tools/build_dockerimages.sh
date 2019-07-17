#!/bin/bash

while getopts ":p:" opt; do
  case $opt in
    p) DOCKER_PUSH="$OPTARG"
    ;;
    \?) echo "Invalid option -$OPTARG" >&2
    ;;
  esac
done

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

CURRENT_DIR=$(pwd)

function build_binary_and_docker {
    BRANCH="${1}"
    REPO="${GOPATH}/src/github.com/elastos/${2}"
    WORKDIR="${3}"
    BINARY="${4}"
    DOCKERIMAGE="${5}"
    DOCKER_PUSH_ALTTAG="${6}"
    DOCKER_PUSH_LATEST="${7}"
    GITHUB_PULL="${8}"

    cd $REPO
    if [ "${GITHUB_PULL}" == "yes" ]
    then
        git checkout master
        git pull
        git checkout $BRANCH
        git pull
    fi
    mkdir -p $TMPDIR/$WORKDIR/$BINARY
    cp -r $CURRENT_DIR/$WORKDIR/* $TMPDIR/$WORKDIR/
    cp -r * $TMPDIR/$WORKDIR/$BINARY/
    docker build -t "$DOCKERIMAGE:latest" -f $TMPDIR/$WORKDIR/Dockerfile $TMPDIR/$WORKDIR/
    if [ "${DOCKER_PUSH}" == "yes" ]
    then
        if [ ! -z "${DOCKER_PUSH_ALTTAG}" ]
        then
            docker tag "$DOCKERIMAGE:latest" "$DOCKERIMAGE:$DOCKER_PUSH_ALTTAG"
            docker push "$DOCKERIMAGE:$DOCKER_PUSH_ALTTAG"
        fi
        if [ "${DOCKER_PUSH_LATEST}" == "yes" ]
        then
            docker push "$DOCKERIMAGE:latest"
        fi
    fi
    cd $CURRENT_DIR
}

function build_docker {
    WORKDIR="${1}"
    BINARY="${2}"
    DOCKERIMAGE="${3}"
    DOCKER_PUSH_ALTTAG="${4}"
    DOCKER_PUSH_LATEST="${5}"

    mkdir -p $TMPDIR/$WORKDIR/$BINARY
    cp -r $CURRENT_DIR/$WORKDIR/* $TMPDIR/$WORKDIR/
    docker build -t "$DOCKERIMAGE:latest" -f $TMPDIR/$WORKDIR/Dockerfile $TMPDIR/$WORKDIR/
    if [ "${DOCKER_PUSH}" == "yes" ]
    then
        if [ ! -z "${DOCKER_PUSH_ALTTAG}" ]
        then
            docker tag "$DOCKERIMAGE:latest" "$DOCKERIMAGE:$DOCKER_PUSH_ALTTAG"
            docker push "$DOCKERIMAGE:$DOCKER_PUSH_ALTTAG"
        fi
        if [ "${DOCKER_PUSH_LATEST}" == "yes" ]
        then
            docker push "$DOCKERIMAGE:latest"
        fi
    fi
    cd $CURRENT_DIR
}

: '
build_docker "ela-sidechain/eth/oracle" "oracle" \
    "cyberrepublic/elastos-sidechain-eth-oracle" "privnet-v0.4" "no"
'

build_binary_and_docker "dev" "Elastos.ELA.SideChain.ETH" "ela-sidechain/eth" "eth" \
    "cyberrepublic/elastos-sidechain-eth-node" "privnet-v0.4" "no" "no"

: '
build_binary_and_docker "v0.3.3" "Elastos.ELA" "ela-mainchain" "ela" \
    "cyberrepublic/elastos-mainchain-node" "v0.3.3" "yes" "yes"

build_binary_and_docker "v0.1.1" "Elastos.ELA.Arbiter" "ela-arbitrator" "arbiter" \
    "cyberrepublic/elastos-arbitrator-node" "v0.1.1" "yes" "yes"

build_binary_and_docker "v0.1.2" "Elastos.ELA.SideChain.ID" "ela-sidechain/did" "did" \
    "cyberrepublic/elastos-sidechain-did-node" "v0.1.2" "yes" "yes"

build_binary_and_docker "v0.1.2" "Elastos.ELA.SideChain.Token" "ela-sidechain/token" "token" \
    "cyberrepublic/elastos-sidechain-token-node" "v0.1.2" "yes" "yes"

build_docker "ela-sidechain/eth" "eth" \
    "cyberrepublic/elastos-sidechain-eth-node" "privnet-v0.4" "no" "yes"

build_binary_and_docker "master" "Elastos.ORG.Wallet.Service" "restful-services/wallet-service" "service" \
    "cyberrepublic/elastos-wallet-service" "privnet-v0.4" "no" "yes"

build_binary_and_docker "master" "Elastos.ORG.SideChain.Service" "restful-services/sidechain-service" "service" \
    "cyberrepublic/elastos-sidechain-service" "privnet-v0.4" "no" "yes"

build_binary_and_docker "master" "Elastos.ORG.API.Misc" "restful-services/api-misc" "misc" \
    "cyberrepublic/elastos-api-misc-service" "privnet-v0.4" "no" "yes"
'

cd $CURRENT_DIR
