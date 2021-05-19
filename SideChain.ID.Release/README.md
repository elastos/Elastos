Elastos ELA SideChain ID
===========
[![Build Status](https://travis-ci.org/elastos/Elastos.ELA.SideChain.ID.svg?branch=master)](https://travis-ci.org/elastos/Elastos.ELA.SideChain.ID) [![Go Report Card](https://goreportcard.com/badge/github.com/elastos/Elastos.ELA.SideChain.ID)](https://goreportcard.com/report/github.com/elastos/Elastos.ELA.SideChain.ID)

## Introduction

Elacoin is the digital currency solution within Elastos eco system.

This project is the source code that can build a full node of DID.


## Table of Contents
- [Elastos ELA SideChain ID](#elastos-ela-sidechain-id)
    - [Introduction](#introduction)
    - [Table of Contents](#table-of-contents)
- [Pre-requisites on Mac](#pre-requisites-on-mac)
    - [1. Check OS version](#1-check-os-version)
    - [2. Install Go distribution 1.13](#2-install-go-distribution-113)
    - [3. Check Go version](#3-check-go-version)
- [Pre-requisites on Ubuntu](#pre-requisites-on-ubuntu)
    - [1. Check Ubuntu version](#1-check-ubuntu-version)
    - [2. Install git](#2-install-git)
    - [3. Install Go distribution 1.13](#3-install-go-distribution-113)
- [Configure the node](#configure-the-node)
- [Build the node](#build-the-node)
    - [1. Check Go version](#1-check-go-version)
    - [2. Clone source code ](#2-clone-source-code)
    - [3. Make](#3-make)
    - [4. Run the node](#4-run-the-node)
- [Interact with the node](#interact-with-the-node)
    - [1. JSON RPC API of the node](#1-json-rpc-api-of-the-node)
- [Contribution](#contribution)
- [Acknowledgments](#acknowledgments)
- [License](#license)

## Pre requisites
#### 1. Check OS version

Make sure the OSX version is 16.7+

```shell
$ uname -srm
Darwin 16.7.0 x86_64
```

#### 2. Install Go distribution 1.13

Use Homebrew to install Golang 1.13.

```shell
$ brew install go@1.13
$ go version

```

#### 3. Check Go version
Check the golang version. Make sure they are the following version number or above.

```shell
$ go version
go version go1.13 darwin/amd64
```

## Pre-requisites on Ubuntu

#### 1. Check Ubuntu version

Make sure your ubuntu version is 16.04+

```shell
$ cat /etc/issue
Ubuntu 16.04.3 LTS \n \l
```

#### 2. Install git

```shell
$ sudo apt-get install -y git
```

#### 3. Install Go distribution 1.13

```shell
$ curl -O https://storage.googleapis.com/golang/go1.13.5.linux-amd64.tar.gz
$ tar -xvf go1.13.5.linux-amd64.tar.gz
$ sudo chown -R root:root ./go
$ sudo mv go /usr/local
$ export GOPATH=$HOME/go
$ export PATH=$PATH:/usr/local/go/bin:$GOPATH/bin
$ source ~/.profile
```

## Configure the node

See the [documentation](./docs/config.json.md) about config.json to understand what each parameter means on the configuration file.

If you would like to connect to testnet, do the following:
```shell
$ cp docs/testnet_config.json.sample config.json
```

If you would like to connect to mainnet, do the following:
```shell
$ cp docs/mainnet_config.json.sample config.json
```

Make sure to modify the parameters to what your own specification. 

## Build the node

#### 1. Check Go version

Check the golang version. Make sure they are the following version number or above.

```shell
$ go version
go version go1.13.5 linux/amd64
```

If you cannot see the version number, there must be something wrong when install.


#### 2. Clone source code
```shell
$ git clone https://github.com/elastos/Elastos.ELA.SideChain.ID.git
```

#### 3. Make

Build the node.
```shell
$ cd Elastos.ELA.SideChain.ID
$ make
```

If you did not see any error message, congratulations, you have made the DID full node.

#### 4. Run the node

Run the node.
```shell
$ ./did
```

## Interact with the node

#### 1. JSON RPC API of the node

Once the node is running successfully, you can access ELA Node's JSON RPC APIs:

Example 1: Get the hash of the most recent block
```bash
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getbestblockhash"}' http://localhost:20606
{
    "error": null,
    "id": null,
    "jsonrpc": "2.0",
    "result": "c4e72359cbb128bca244a800fb36d71f64b834e20d437c25de6c62edc46196c7"
}
```

Example 2: Get the hash of the specific blockchain height
```bash
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getblockhash","params":{"height":1}}' http://localhost:20606
{
    "error": null,
    "id": null,
    "jsonrpc": "2.0",
    "result": "71b422e09dcd2f749d2adc0086735c210084cdb6b59bd4cd42e50455d024a662"
}
```

If you would like to learn more about what other JSON RPC APIs are available for the node, please check out the [JSON RPC API](docs/jsonrpc_apis.md)

## Contribution

We welcome contributions to the Elastos ELA SideChain ID Project.

## Acknowledgments

A sincere thank you to all teams and projects that we rely on directly or indirectly.

## License 

This project is licensed under the terms of the [MIT license](https://github.com/elastos/Elastos.ELA.SideChain.ID/blob/master/LICENSE).
