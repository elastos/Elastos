#!/bin/bash

export GOARCH="amd64"
export GOOS="linux"
export CGO_ENABLED=0

CURRENT_DIR=$(pwd)

# Helper function to build the binary for blockchain related repos
function blockchain_build_binary {
    BRANCH="${1}"
    REPO="${GOPATH}/src/github.com/elastos/${2}"

    cd $REPO
    git checkout master
    git pull
    git checkout $BRANCH
    rm -rf vendor glide.lock
    glide update && glide install
    make
}

# Build github.com/elastos/Elastos.ELA
blockchain_build_binary "release_v0.3.2" "Elastos.ELA" 
xargs -n 1 cp -v ela<<<"$CURRENT_DIR/ela-mainchain/node_normal/mainchain-normal-1/  \
                        $CURRENT_DIR/ela-mainchain/node_dpos/mainchain-dpos-1/      \
                        $CURRENT_DIR/ela-mainchain/node_dpos/mainchain-dpos-2/      \
                        $CURRENT_DIR/ela-mainchain/node_crc/mainchain-crc-1/        \
                        $CURRENT_DIR/ela-mainchain/node_crc/mainchain-crc-2/        \
                        $CURRENT_DIR/ela-mainchain/node_crc/mainchain-crc-3/        \
                        $CURRENT_DIR/ela-mainchain/node_crc/mainchain-crc-4/"
cp ela-cli $CURRENT_DIR/tools/

# Build github.com/elastos/Elastos.ELA.Arbiter
blockchain_build_binary "release_v0.1.1" "Elastos.ELA.Arbiter" 
xargs -n 1 cp -v arbiter<<<"$CURRENT_DIR/ela-arbitrator/node_origin/arbitrator-origin-1/    \
                            $CURRENT_DIR/ela-arbitrator/node_origin/arbitrator-origin-2/    \
                            $CURRENT_DIR/ela-arbitrator/node_crc/arbitrator-crc-1/          \
                            $CURRENT_DIR/ela-arbitrator/node_crc/arbitrator-crc-2/          \
                            $CURRENT_DIR/ela-arbitrator/node_crc/arbitrator-crc-3/          \
                            $CURRENT_DIR/ela-arbitrator/node_crc/arbitrator-crc-4/"

# Build github.com/elastos/Elastos.ELA.SideChain.ID
blockchain_build_binary "release_v0.1.2" "Elastos.ELA.SideChain.ID" 
xargs -n 1 cp -v did<<<"$CURRENT_DIR/ela-sidechain/did/did-1/   \
                        $CURRENT_DIR/ela-sidechain/did/did-2/   \
                        $CURRENT_DIR/ela-sidechain/did/did-3/   \
                        $CURRENT_DIR/ela-sidechain/did/did-4/"

# Build github.com/elastos/Elastos.ELA.SideChain.Token
blockchain_build_binary "release_v0.1.1" "Elastos.ELA.SideChain.Token" 
xargs -n 1 cp -v token<<<"$CURRENT_DIR/ela-sidechain/token/token-1/   \
                          $CURRENT_DIR/ela-sidechain/token/token-2/   \
                          $CURRENT_DIR/ela-sidechain/token/token-3/   \
                          $CURRENT_DIR/ela-sidechain/token/token-4/"

# Helper function to build the jars for REST API services
function services_build_jars {
    BRANCH="${1}"
    REPO="${GOPATH}/src/github.com/elastos/${2}"
    PREFIX="${3}"
    OLD_NODE_INFO="${4}"
    NEW_NODE_INFO="${5}"
    NEW_PORT="${6}"

    cd $REPO
    git checkout master
    git pull
    git checkout $BRANCH
    cat "${PREFIX}.api/src/main/resources/application.properties.in"    \
        | sed "s#${OLD_NODE_INFO}#${NEW_NODE_INFO}#g"                   \
        | sed "s#8080#${NEW_PORT}#g"                                           \
        > "${PREFIX}.api/src/main/resources/application.properties"
    mvn clean
    mvn install -Dmaven.test.skip -Dgpg.skip
    cp "${PREFIX}.api/target/${PREFIX}.api-0.0.6.jar" $CURRENT_DIR/restful-services/wallet-service/
}

# Build github.com/elastos/Elastos.ORG.Wallet.Service
services_build_jars "master" "Elastos.ORG.Wallet.Service" "base" \
        "@ELA_REST_API@" "http://ela-mainchain-normal-1:10012" "8091"

# Build github.com/elastos/Elastos.ORG.DID.Service
services_build_jars "master" "Elastos.ORG.DID.Service" "did" \
        "@SIDE_REST_API@" "http://ela-sidechain-did-1:10032" "8092"

# Build github.com/elastos/Elastos.ORG.API.Misc 
BRANCH="master"
REPO="${GOPATH}/src/github.com/elastos/Elastos.ORG.API.Misc"
cd $REPO
git checkout master
git pull
git checkout $BRANCH
rm -rf vendor glide.lock
glide update && glide install
go build -o misc
cp misc $CURRENT_DIR/restful-services/api-misc/

cd $CURRENT_DIR