#!/bin/bash

PRIVNET_TAG="privnet-v0.6"

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
    DOCKERIMAGE="${4}"
    GITHUB_PULL="${5}"
    DOCKER_PUSH_TAG_BRANCH="${6}"
    DOCKER_PUSH_TAG_PRIVNET="${7}"

    cd $REPO
    if [ "${GITHUB_PULL}" == "yes" ]
    then
        git checkout master
        git pull
        git checkout $BRANCH
        git pull
    fi
    mkdir -p $TMPDIR/$WORKDIR
    cp -r * $TMPDIR/$WORKDIR/
    docker build -t "$DOCKERIMAGE:latest" -f $CURRENT_DIR/docker/$WORKDIR.Dockerfile $TMPDIR/
    if [ "${DOCKER_PUSH}" == "yes" ]
    then
        if [ ! -z "${DOCKER_PUSH_TAG_BRANCH}" ]
        then
            docker tag "$DOCKERIMAGE:latest" "$DOCKERIMAGE:$DOCKER_PUSH_TAG_BRANCH"
            docker push "$DOCKERIMAGE:$DOCKER_PUSH_TAG_BRANCH"
        fi
        if [ "${DOCKER_PUSH_TAG_PRIVNET}" == "yes" ]
        then
            docker tag "$DOCKERIMAGE:latest" "$DOCKERIMAGE:$PRIVNET_TAG"
            docker push "$DOCKERIMAGE:$PRIVNET_TAG"
        fi
        docker push "$DOCKERIMAGE:latest"
    fi
    cd $CURRENT_DIR
}

function build_docker {
    WORKDIR="${1}"
    DOCKERFILE_PREFIX="${2}"
    DOCKERIMAGE="${3}"
    DOCKER_PUSH_TAG_BRANCH="${4}"
    DOCKER_PUSH_TAG_PRIVNET="${5}"

    mkdir -p $TMPDIR/$DOCKERFILE_PREFIX
    cp -r $CURRENT_DIR/docker/$WORKDIR/* $TMPDIR/$DOCKERFILE_PREFIX/
    docker build -t "$DOCKERIMAGE:latest" -f $CURRENT_DIR/docker/$DOCKERFILE_PREFIX.Dockerfile $TMPDIR/
    if [ "${DOCKER_PUSH}" == "yes" ]
    then
        if [ ! -z "${DOCKER_PUSH_TAG_BRANCH}" ]
        then
            docker tag "$DOCKERIMAGE:latest" "$DOCKERIMAGE:$DOCKER_PUSH_TAG_BRANCH"
            docker push "$DOCKERIMAGE:$DOCKER_PUSH_TAG_BRANCH"
        fi
        if [ "${DOCKER_PUSH_TAG_PRIVNET}" == "yes" ]
        then
            docker tag "$DOCKERIMAGE:latest" "$DOCKERIMAGE:$PRIVNET_TAG"
            docker push "$DOCKERIMAGE:$PRIVNET_TAG"
        fi
        docker push "$DOCKERIMAGE:latest"
    fi
    cd $CURRENT_DIR
}

build_binary_and_docker "v0.3.7" "Elastos.ELA" "ela" \
    "cyberrepublic/elastos-mainchain-node" "yes" "v0.3.7" "yes"

build_binary_and_docker "v0.1.2" "Elastos.ELA.Arbiter" "arbitrator" \
    "cyberrepublic/elastos-arbitrator-node" "yes" "v0.1.2" "yes"

build_binary_and_docker "release_v0.1.3" "Elastos.ELA.SideChain.ID" "sidechain.did" \
    "cyberrepublic/elastos-sidechain-did-node" "yes" "v0.1.3" "yes"

build_binary_and_docker "v0.1.2" "Elastos.ELA.SideChain.Token" "sidechain.token" \
    "cyberrepublic/elastos-sidechain-token-node" "yes" "v0.1.2" "yes"

build_binary_and_docker "dev" "Elastos.ELA.SideChain.ETH" "sidechain.eth" \
    "cyberrepublic/elastos-sidechain-eth-node" "no" "dev" "yes"

build_docker "sidechain/eth/oracle" "sidechain.eth.oracle" \
    "cyberrepublic/elastos-sidechain-eth-oracle" "v0.0.1" "yes"

build_binary_and_docker "9acddc6e5ce3ffa7305e618b723b66b9edf58108" "Elastos.ORG.Wallet.Service" "service.wallet" \
    "cyberrepublic/elastos-wallet-service" "yes" "" "yes"

build_binary_and_docker "fb82b27b884b1f5dd61f7d0b3278d5d695916b94" "Elastos.ORG.SideChain.Service" "service.sidechain" \
    "cyberrepublic/elastos-sidechain-service" "yes" "" "yes"

build_binary_and_docker "d72f0570cd7990e600a1393cf35cd0907f4dbdd8" "Elastos.ORG.API.Misc" "service.misc" \
    "cyberrepublic/elastos-api-misc-service" "yes" "" "yes"

build_binary_and_docker "257882bb7f52bda10c190a280e82b99e248fb4c6" "Elastos.ELA.Elephant.Node" "elephant" \
    "cyberrepublic/elastos-elephant-node" "yes" "" "yes"

cd $CURRENT_DIR
