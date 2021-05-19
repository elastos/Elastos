#!/bin/bash

PRIVNET_TAG="privnet-v0.7"

while getopts ":p:t:i:h" opt; do
  case $opt in
    h)
      echo "Usage:"
      echo "    tools/build_dockerimages.sh -h                  Display this help message."
      echo "    tools/build_dockerimages.sh -t [yes|no]         Do we also want to put the latest private net tag to the docker images?"
      echo "    tools/build_dockerimages.sh -p [yes|no]         Push built images to docker registry. You will need access to push to https://hub.docker.com/u/cyberrepublic to use this option."
      echo "    tools/build_dockerimages.sh -i  [all|ela|arbitrator|sidechain.did|sidechain.token|sidechain.eth|sidechain.eth.oracle|service.wallet|service.sidechain|service.misc|elaphant]         Build a specific docker image."
      exit 0
    ;;
    p) DOCKER_PUSH="$OPTARG"
    ;;
    t) DOCKER_PUSH_TAG_PRIVNET="$OPTARG"
    ;;
    i) DOCKER_IMAGE_TO_BUILD="$OPTARG"
    ;;
    \? )
      echo "Invalid Option: -$OPTARG" 1>&2
      exit 1
    ;;
  esac
done

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

CURRENT_DIR=$(pwd)

function build_binary_and_docker {
    BRANCH="${1}"
    REPO="../../../${2}"
    WORKDIR="${3}"
    DOCKERIMAGE="${4}"
    GITHUB_PULL="${5}"
    DOCKER_PUSH_TAG_BRANCH="${6}"

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
if [ "${DOCKER_IMAGE_TO_BUILD}" == "ela" ] || [ "${DOCKER_IMAGE_TO_BUILD}" == "all" ]
then 
    build_binary_and_docker "v0.5.0" "github.com/elastos/Elastos.ELA" "ela" \
        "cyberrepublic/elastos-mainchain-node" "yes" "v0.5.0"
fi
if [ "${DOCKER_IMAGE_TO_BUILD}" == "arbitrator" ] || [ "${DOCKER_IMAGE_TO_BUILD}" == "all" ]
then 
    build_binary_and_docker "release_v0.1.2" "github.com/elastos/Elastos.ELA.Arbiter" "arbitrator" \
        "cyberrepublic/elastos-arbitrator-node" "yes" "v0.1.2"
fi
if [ "${DOCKER_IMAGE_TO_BUILD}" == "sidechain.did" ] || [ "${DOCKER_IMAGE_TO_BUILD}" == "all" ]
then 
    build_binary_and_docker "v0.1.5" "github.com/elastos/Elastos.ELA.SideChain.ID" "sidechain.did" \
        "cyberrepublic/elastos-sidechain-did-node" "yes" "v0.1.5"
fi
if [ "${DOCKER_IMAGE_TO_BUILD}" == "sidechain.token" ] || [ "${DOCKER_IMAGE_TO_BUILD}" == "all" ]
then 
    build_binary_and_docker "v0.1.2" "github.com/elastos/Elastos.ELA.SideChain.Token" "sidechain.token" \
        "cyberrepublic/elastos-sidechain-token-node" "yes" "v0.1.2"
fi
if [ "${DOCKER_IMAGE_TO_BUILD}" == "sidechain.eth" ] || [ "${DOCKER_IMAGE_TO_BUILD}" == "all" ]
then 
    build_binary_and_docker "v0.0.2" "github.com/elastos/Elastos.ELA.SideChain.ETH" "sidechain.eth" \
        "cyberrepublic/elastos-sidechain-eth-node" "no" "v0.0.2"
fi
if [ "${DOCKER_IMAGE_TO_BUILD}" == "sidechain.eth.oracle" ] || [ "${DOCKER_IMAGE_TO_BUILD}" == "all" ]
then 
    build_docker "sidechain/eth/oracle" "sidechain.eth.oracle" \
        "cyberrepublic/elastos-sidechain-eth-oracle" "v0.0.2"
fi
if [ "${DOCKER_IMAGE_TO_BUILD}" == "service.wallet" ] || [ "${DOCKER_IMAGE_TO_BUILD}" == "all" ]
then 
    build_binary_and_docker "bfd56b6516a85740c020c3e600cbd165780319e7" "github.com/elastos/Elastos.ORG.Wallet.Service" "service.wallet" \
        "cyberrepublic/elastos-wallet-service" "yes" ""
fi
if [ "${DOCKER_IMAGE_TO_BUILD}" == "service.sidechain" ] || [ "${DOCKER_IMAGE_TO_BUILD}" == "all" ]
then 
    build_binary_and_docker "fb82b27b884b1f5dd61f7d0b3278d5d695916b94" "github.com/elastos/Elastos.ORG.SideChain.Service" "service.sidechain" \
        "cyberrepublic/elastos-sidechain-service" "yes" ""
fi
if [ "${DOCKER_IMAGE_TO_BUILD}" == "service.misc" ] || [ "${DOCKER_IMAGE_TO_BUILD}" == "all" ]
then 
    build_binary_and_docker "6da23db74c01baf7a01938d586582e4260036682" "github.com/elastos/Elastos.ORG.API.Misc" "service.misc" \
        "cyberrepublic/elastos-api-misc-service" "yes" ""
fi
if [ "${DOCKER_IMAGE_TO_BUILD}" == "elaphant" ] || [ "${DOCKER_IMAGE_TO_BUILD}" == "all" ]
then 
    build_binary_and_docker "v0.4.0" "github.com/elaphantapp/ElaphantNode" "elaphant" \
        "cyberrepublic/elastos-elaphant-node" "yes" "v0.4.0"
fi

cd $CURRENT_DIR
