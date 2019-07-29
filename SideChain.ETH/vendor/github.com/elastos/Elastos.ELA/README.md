Elastos ELA
===========
|Travis CI|Go Report Card|
|:-:|:-:|
|[![Build Status](https://travis-ci.org/elastos/Elastos.ELA.svg?branch=master)](https://travis-ci.org/elastos/Elastos.ELA)|[![Code Report](https://goreportcard.com/badge/github.com/elastos/Elastos.ELA)](https://goreportcard.com/report/github.com/elastos/Elastos.ELA)|

## Introduction

ELA is the digital currency solution within the Elastos ecosystem. It is merged mined with Bitcoin which means the existing bitcoin miners are able to merge mine both BTC and ELA at the same time without expending any additional resources or energy while also providing the enormous hashpower that comes with the bitcoin network.

This project is the source code that can build a full node of ELA blockchain(main chain). 

## Table of Contents
- [Elastos ELA](#elastos-ela)
    - [Introduction](#introduction)
    - [Table of Contents](#table-of-contents)
- [Pre-requisites on Mac](#pre-requisites-on-mac)
    - [1. Check OS version](#1-check-os-version)
    - [2. Install Go distribution 1.9](#2-install-go-distribution-19)
    - [3. Install Glide](#3-install-glide)
- [Pre-requisites on Ubuntu](#pre-requisites-on-ubuntu)
    - [1. Check OS version](#1-check-os-version-1)
    - [2. Install git](#2-install-git)
    - [3. Install Go distribution 1.9](#3-install-go-distribution-19)
    - [4. Install Glide](#4-install-glide)
- [Configure the node](#configure-the-node)
- [Build the node](#build-the-node)
    - [1. Setup basic workspace](#1-setup-basic-workspace)
    - [2. Set correct environment variables](#2-set-correct-environment-variables)
    - [3. Check Go version and glide version](#3-check-go-version-and-glide-version)
    - [4. Clone source code to $GOPATH/src/github/elastos folder](#4-clone-source-code-to-gopathsrcgithubelastos-folder)
    - [5. Install dependencies using Glide](#5-install-dependencies-using-glide)
    - [6. Make](#6-make)
    - [7. Run the node on Mac](#7-run-the-node-on-mac)
- [Build and Run using docker](#build-and-run-using-docker)
    - [1. Build the docker node](#1-build-the-docker-node)
    - [2. Run the node in the docker container](#2-run-the-node-in-the-docker-container)
- [Interact with the node](#interact-with-the-node)
    - [1. Access the web UI of the node](#1-access-the-web-ui-of-the-node)
    - [2. REST API of the node](#2-rest-api-of-the-node)
    - [3. JSON RPC API of the node](#3-json-rpc-api-of-the-node)
- [Contribution](#contribution)
- [Acknowledgments](#acknowledgments)
- [License](#license)

## Pre-requisites on Mac

#### 1. Check OS version

Make sure the OSX version is 16.7+

```shell
$ uname -srm
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
$ cd ~/dev
$ curl https://glide.sh/get | sh
```

## Configure the node

You can just run a `ela` node without a `config.json` file, the `ela` node will use the main net configuration by default, and provide a JSON-RPC service on `localhost:20336`.

If you want to customize the node configuration, see the [`config.json`](./docs/config.json.md) to understand what each parameter means on the configuration file.

If you would like to connect to testnet, do the following:
```shell
$ cp docs/testnet_config.json.sample config.json
```

If you would like a simple config template, do the following:
```shell
$ cp docs/mainnet_config.json.sample config.json
```

Make sure to modify the parameters to what your own specification. 

## Build the node

#### 1. Setup basic workspace
In this instruction we use ~/dev/src/github.com/elastos as our working directory. If you clone the source code to a different directory, please make sure you change other environment variables accordingly (not recommended). 

```shell
$ mkdir -p ~/dev/bin
$ mkdir -p ~/dev/src/github.com/elastos/
```

#### 2. Set correct environment variables

```shell
export GOROOT=/usr/local/opt/go@1.9/libexec
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

If you cannot see the version number, there must be something wrong when install.

#### 4. Clone source code to $GOPATH/src/github/elastos folder
Make sure you are in the folder of $GOPATH/src/github.com/elastos
```shell
$ git clone https://github.com/elastos/Elastos.ELA.git
```

If clone works successfully, you should see folder structure like $GOPATH/src/github.com/elastos/Elastos.ELA/Makefile

#### 5. Install dependencies using Glide

```shell
$ cd $GOPATH/src/github.com/elastos/Elastos.ELA
$ glide update && glide install
``` 

#### 6. Make

Build the node.
```shell
$ cd $GOPATH/src/github.com/elastos/Elastos.ELA
$ make
```

If you did not see any error message, congratulations, you have made the ELA full node.

#### 7. Run the node on Mac

Run the node.
```shell
$ ./ela
```

## Build and Run using docker

Alternatively, if don't want to build it manually on Mac or Linux, we also provide a `Dockerfile` to help you (You need to have docker installed).

#### 1. Build the docker node

```shell
$ cd docker
$ docker build -t ela_node_run .
```

#### 2. Run the node in the docker container

```shell
$ docker run -p 20334:20334 -p 20335:20335 -p 20336:20336 -p 20338:20338 ela_node_run
```

> Note: Don't hit Ctrl-C to terminate the output; instead close this terminal and open another.

> Please note the dockerfile uses the default 'config.json' in the repository. If you're familiar with docker, you can change the docker file to make it use your own ELA Node configuration file.

## Interact with the node

#### 1. Access the web UI of the node

If you would like to access the web UI of the node to get different stats about the node, go to the following URL on your browser: `http://localhost:21333/info`

#### 2. REST API of the node

Once the node is running successfully, you can access ELA Node's REST APIs:

Example 1: Get the number of nodes to which the node is connected
```bash
curl http://localhost:21334/api/v1/node/connectioncount
{
    "Desc": "Success",
    "Error": 0,
    "Result": 5
}
```
Example 2: Get the block height of the node
```bash
curl http://localhost:21334/api/v1/block/height
{
    "Desc": "Success",
    "Error": 0,
    "Result": 1000
}
```

If you would like to learn more about what other REST APIs are available for the node, please check out the [Restful API](docs/Restful_API.md)

#### 3. JSON RPC API of the node

Once the node is running successfully, you can access ELA Node's JSON RPC APIs:

Example 1: Get the hash of the most recent block
```bash
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getbestblockhash"}' http://localhost:21336
{
    "error": null,
    "id": null,
    "jsonrpc": "2.0",
    "result": "c4e72359cbb128bca244a800fb36d71f64b834e20d437c25de6c62edc46196c7"
}
```

Example 2: Get the hash of the specific blockchain height
```bash
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getblockhash","params":{"height":1}}' http://localhost:21336
{
    "error": null,
    "id": null,
    "jsonrpc": "2.0",
    "result": "71b422e09dcd2f749d2adc0086735c210084cdb6b59bd4cd42e50455d024a662"
}
```

If you would like to learn more about what other JSON RPC APIs are available for the node, please check out the [JSON RPC API](docs/jsonrpc_apis.md)

## Contribution

We welcome contributions to the Elastos ELA Project.

## Acknowledgments

A sincere thank you to all teams and projects that we rely on directly or indirectly.

## License 

This project is licensed under the terms of the [MIT license](https://github.com/elastos/Elastos.ELA/blob/master/LICENSE).
