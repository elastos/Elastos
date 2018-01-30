#!/bin/bash

#
# Utility
#
install_package_ubuntu()
{
    local PKG_NAME=$1
    local PPA_NAME=$2

    if [ "$PKG_NAME" == "" ]; then
        echo "ERROR: no package name specified"
        return
    fi

    dpkg -V $PKG_NAME 2>/dev/null
    if [ "$?" == "0" ]; then
        return
    fi

    if [ "$PPA_NAME" != "" ]; then
        sudo add-apt-repository -y -r $PPA_NAME
        sudo add-apt-repository -y $PPA_NAME
        sudo apt-get update -q
    fi

    sudo apt-get install -y $PKG_NAME
}


install_package_macosx()
{
    local PKG_NAME=$1
    local BREW_INSTALL_OPTION=$2

    if [ "$PKG_NAME" == "" ]; then
        echo "ERROR: no package name specified"
        return
    fi

    which brew 1>/dev/null 2>/dev/null
    if [ "$?" != "0" ]; then
        /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
    fi

    brew list $PKG_NAME 1>/dev/null 2>/dev/null
    if [ "$?" == "0" ]; then
        return
    fi
    brew install $BREW_INSTALL_OPTION $PKG_NAME
}

#
# Development Environment: Directory Layout
#
# $HOME
#  dev/         - DEV_ROOT - GOPATH
#    src/       - SRC_ROOT
#      ELA/ - SRC_PATH
#        deps/  - Depended C source code
#
setenv()
{
    #
    # Check OS version
    #
    OS_NAME=$(uname -s)
    if [ "$OS_NAME" == "Linux" ]; then
        OS_DETAIL="$(lsb_release -i -s) $(lsb_release -r -s) $(uname -m)"
    elif [ "$OS_NAME" == "Darwin" ]; then
        OS_DETAIL="$(uname -srm)"
    fi

    if [ "$OS_DETAIL" == "Ubuntu 16.04 x86_64" ]; then
        echo "$OS_DETAIL: Supported"
    elif [ "$OS_DETAIL" == "Darwin 16.7.0 x86_64" ]; then
        echo "$OS_DETAIL: Supported"
    else
        echo "ERROR: $OS_DETAIL have not been tested"
        exit
    fi

    if [ "$OS_NAME" == "Linux" ]; then
        install_package_ubuntu git
        install_package_ubuntu software-properties-common
        install_package_ubuntu golang-1.8-go ppa:gophers/archive
        install_package_ubuntu glide ppa:masterminds/glide
    elif [ "$OS_NAME" == "Darwin" ]; then
        install_package_macosx go@1.8
        install_package_macosx glide --ignore-dependencies
        install_package_macosx pkg-config
        install_package_macosx zeromq@4.2
    fi

    export SCRIPT_PATH=$(cd $(dirname $BASH_SOURCE); pwd)
    export SRC_PATH=$SCRIPT_PATH

    export DEV_ROOT=$(cd $SCRIPT_PATH/../..; pwd)
    export SRC_ROOT=$DEV_ROOT/src

    if [ "$OS_NAME" == "Linux" ]; then
        export GOROOT=/usr/lib/go-1.8
    elif [ "$OS_NAME" == "Darwin" ]; then
        export GOROOT=/usr/local/opt/go@1.8/libexec
    fi

    export GOPATH=$DEV_ROOT
    export PATH=$GOROOT/bin:$PATH

    go version
    glide --version

    NCPU=1
    if [ "$(uname -s)" == "Linux" ]; then
        NCPU=$(($(grep '^processor' /proc/cpuinfo | wc -l) * 2))
    elif [ "$(uname -s)" == "Darwin" ]; then
        NCPU=$(($(sysctl -n hw.ncpu) * 2))
    fi
}

build()
{
    cd $SRC_PATH
    glide install
    make $*
}

usage()
{
    echo "Usage: $(basename $0)"
    echo "Build this project"
}

#
# Main
#
if [ "$1" == "-h" ]; then
    usage
    exit
fi

setenv
build $1
