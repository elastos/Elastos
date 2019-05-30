#!/bin/bash

while getopts ":p:l:" opt; do
  case $opt in
    p) DOCKER_PUSH="$OPTARG"
    ;;
    l) DOCKER_PUSH_LATEST="$OPTARG"
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

    cd $REPO
    git checkout $BRANCH
    git pull
    mkdir -p $TMPDIR/$WORKDIR/$BINARY
    cp $CURRENT_DIR/$WORKDIR/* $TMPDIR/$WORKDIR/
    cp -r * $TMPDIR/$WORKDIR/$BINARY/
    docker build -t "$DOCKERIMAGE:latest" -f $TMPDIR/$WORKDIR/Dockerfile $TMPDIR/$WORKDIR/
    if [ "${DOCKER_PUSH}" == "yes" ]
    then
        docker tag "$DOCKERIMAGE:latest" "$DOCKERIMAGE:$BRANCH"
        docker push "$DOCKERIMAGE:$BRANCH"
        if [ "${DOCKER_PUSH_LATEST}" == "yes" ]
        then
            docker push "$DOCKERIMAGE:latest"
        fi
    fi
    cd $CURRENT_DIR
}

build_binary_and_docker "release-v5.2" "Elastos.NET.Carrier.Bootstrap" "bootstrap" "bootstrapd" \
    "cyberrepublic/elastos-carrier-bootstrap-node"

cd $CURRENT_DIR
