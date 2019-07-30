#!/bin/bash

PRIVNET_TAG="privnet-v0.4"

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
    GITHUB_PULL="${6}"
    DOCKER_PUSH_TAGS="${7}"

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
        if [ ! -z "${DOCKER_PUSH_TAGS}" ]
        then
            docker tag "$DOCKERIMAGE:latest" "$DOCKERIMAGE:$BRANCH"
            docker push "$DOCKERIMAGE:$BRANCH"
            docker tag "$DOCKERIMAGE:latest" "$DOCKERIMAGE:$PRIVNET_TAG"
            docker push "$DOCKERIMAGE:$PRIVNET_TAG"
        fi
        docker push "$DOCKERIMAGE:latest"
    fi
    cd $CURRENT_DIR
}

build_binary_and_docker "release-v5.2.3" "Elastos.NET.Carrier.Bootstrap" "bootstrap" "bootstrapd" \
    "cyberrepublic/elastos-carrier-bootstrap-node" "yes" "yes"

cd $CURRENT_DIR
