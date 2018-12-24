# Elastos.ELA.SideChain.NeoVM

## Introduction
This project is based by Elastos.ELA.SideChain.  
It integrated neo's smart contract VM,can run neo's smart contract.

## Table of Contents

- [Introduction](#introduction)

- [Pre-requisites on Mac](#pre-requisites-on-mac)

  - [1. Check OS version](#1-check-os-version)

  - [2. Install Go distribution 1.9](#2-install-go-distribution-19)

  - [3. Install Glide](#3-install-glide)

- [Pre-requisites on Ubuntu](#pre-requisites-on-ubuntu)

  - [1. Check OS version](#1-check-os-version-1)

  - [2.Install git](#2-install-git)

  - [3. Install Go distribution 1.9](#3-install-go-distribution-19)

  - [4. Install Glide](#4-install-glide)

- [Configure the node](#configure-the-node)

- [Build the node](#build-the-node)

  - [1. Setup basic workspace](#1-setup-basic-workspace)
  
  - [2. Set correct environment variables](#2-set-correct-environment-variables)
  
  - [3. Check Go version and glide version](#3-check-go-version-and-glide-version)
  
  - [4. Clone source code to $GOPATH/src/github/elastos folder](#4-clone-source-code-to-gopathsrcgithubcomelastos-folder)
  
  - [5. Install dependencies using Glide](#5-install-dependencies-using-glide)
  
  - [6. Make](#6-make)
  
  - [7. Run the node](#7-run-the-node)

- [License](#license)

## Pre-requisites on Mac

#### 1. Check OS version
Make sure the OSX version is 16.7+

```shell
uname -srm
Darwin 16.7.0 x86_64
```  
#### 2. Install Go distribution 1.9
Use Homebrew to install Golang 1.9.

```shell
$ brew install go@1.9
```
> If you install older version, such as v1.8, you may get missing math/bits package error when build.

#### 3. Install Glide

Glide is a package manager for Golang. We use Glide to install dependent packages.

```shell
$ brew install --ignore-dependencies glide
```

## Pre-requisites on Ubuntu

#### 1. Check OS version

Make sure your ubuntu version is 16.04+

```shell
$ cat /etc/issue
Ubuntu 16.04.3 LTS \n \l
```

#### 2. Install git

```shell
$ sudo apt-get install -y git
```
#### 3. Install Go distribution 1.9

```shell
$ sudo apt-get install -y software-properties-common
$ sudo add-apt-repository -y ppa:gophers/archive
$ sudo apt update
$ sudo apt-get install -y golang-1.9-go
```

> If you install older version, such as v1.8, you may get missing math/bits package error when build.

#### 4. Install Glide

Glide is a package manager for Golang. We use Glide to install dependent packages.

```shell
$ cd
$ mkdir dev
$ cd dev
$ curl https://glide.sh/get | sh
```
If installation glide report **glide needs envirable $GOPATH. set it befor continue**. then go to [Build the node](#4),to set the environment variable

## Configure the node

See the [documentation](./docs/config.json.md) about config.json

If yo would like to connect to testnet, do the following:

```shell
cp docs/testnet_config.json.sample config.json
```

If you would like to connect to mainnet, do the following:

```shell
cp docs/mainnet_config.json.sample config.json
```

Make sure to modify the parameters to what your own specification

## Build the node

#### 1. Setup basic workspace
In this instruction we use ~/dev/src/github.com/elastos as our working directory. If you clone the source code to a different directory, please make sure you change other environment variables accordingly (not recommended).

```shell
$ mkdir -p ~/dev/bin
$ mkdir -p ~/dev/src/github.com/elastos/
```

#### 2. Set correct environment variables
  Mac OS:
  
```shell
export GOROOT=/usr/local/opt/go@1.9/libexec
export GOPATH=$HOME/dev
export GOBIN=$GOPATH/bin
export PATH=$GOROOT/bin:$PATH
export PATH=$GOBIN:$PATH
```

  Ubuntu OS:

```shell
export GOROOT=/usr/lib/go-1.9
export GOPATH=$HOME/dev
export GOBIN=$GOPATH/bin
export PATH=$GOROOT/bin:$PATH
export PATH=$GOBIN:$PATH
```


#### 3. Check Go version and glide version

Check the golang and glider version. Make sure they are the following version number or above.

```shell
$ go version
go version go1.9.2 darwin/amd64

$ glide --version
glide version 0.13.1
```

#### 4. Clone source code to $GOPATH/src/github.com/elastos folder

Make sure you are in the folder of $GOPATH/src/github.com/elastos folder

```shell
$ git clone https://github.com/elastos/Elastos.ELA.SideChain.NeoVM.git

```

If clone works successfully, you should see folder structure like $GOPATH/src/github.com/elastos/Elastos.ELA.SideChain.NeoVM/Makefile

#### 5. Install dependencies using Glide

```shell
$ cd $GOPATH/src/github.com/elastos/Elastos.ELA.SideChain.NeoVM
$ glide update && glide install
```
If the terminal report "Cannot detect VCS", we need to set mirror, please search it on web.

#### 6. Make

Build the node

```shell
$ cd $GOPATH/src/github.com/elastos/Elastos.ELA.SideChain.NeoVM
$ make
```

If you did not see any error message, congratulations, you have made the NeoVM full node.

#### 7. Run the node

```shell
$ ./neo
```
## License

This project is licensed under the terms of the [MIT license](https://github.com/elastos/Elastos.ELA/blob/master/LICENSE).