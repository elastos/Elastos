#Elastos.ELA.SideChain.NeoVM

##<h2 id = "0">Introduction
this project is based by Elastos.ELA.SideChain, it integrated Neo's smart contract VM,can run neo's smart contract.

## Table of Contents

- [Introduction](#0)

- [Pre-requisites on Mac](#1)

  - [1. Check OS version](#1.1)

  - [2. Install Go distribution 1.9](#1.2)

  - [3. Install Glide](#1.3)

- [Pre-requisites on Ubuntu](#2)

  - [1. Check OS version](#2.1)

  - [2.Install git](#2.2)

  - [3. Install Go distribution 1.9](#2.3)

  - [4. Install Glide](#2.4)

- [Configure the node](#3)

- [Build the node](#4)

  - [1. Setup basic workspace](#4.1)
  
  - [2. Set correct environment variables](#4.2)
  
  - [3. Check Go version and glide version](#4.3)
  
  - [4. Clone source code to $GOPATH/src/github/elastos folder](#4.4)
  
  - [5. Install dependencies using Glide](#4.5)
  
  - [6. Make](#4.6)
  
  - [7. Run the node](#4.7)

  
- [Build and Run using docker](#5)
  - [1. Build the docker node](#5.1)

  - [2. Run the node in the docker container](#5.2)

- [License](#6)

##<h2 id = "1">Pre-requisites on Mac

#### <h4 id = "1.1">1. Check OS version
Make sure the OSX version is 16.7+

```shell
uname -srm
Darwin 16.7.0 x86_64
```
####<h4 id = "1.2">2. Install Go distribution 1.9

Use Homebrew to install Golang 1.9.

```shell
$ brew install go@1.9
```
> If you install older version, such as v1.8, you may get missing math/bits package error when build.

####<h4 id = "1.3">3. Install Glide

Glide is a package manager for Golang. We use Glide to install dependent packages.

```shell
$ brew install --ignore-dependencies glide
```

##<h2 id = "2">Pre-requisites on Ubuntu

#### <h4 id = "2.1">1. Check OS version

Make sure your ubuntu version is 16.04+

```shell
$ cat /etc/issue
Ubuntu 16.04.3 LTS \n \l
```

#### <h4 id = "2.2">2. Install git

```shell
$ sudo apt-get install -y git
```
#### <h4 id = "2.3">3.Install Go distribution 1.9

```shell
$ sudo apt-get install -y software-properties-common
$ sudo add-apt-repository -y ppa:gophers/archive
$ sudo apt update
$ sudo apt-get install -y golang-1.9-go
```

> If you install older version, such as v1.8, you may get missing math/bits package error when build.

#### <h4 id = "2.4">4. Install Glide

Glide is a package manager for Golang. We use Glide to install dependent packages.

```shell
$ cd
$ mkdir dev
$ cd dev
$ curl https://glide.sh/get | sh
```
if installation glide report **glide needs envirable $GOPATH. set it befor continue**. then go to [Build the node](#4),to set the environment variable

##<h2 id = "3">Configure the node

See the [documentation](./docs/config.json.md) about config.json

Make sure to modify the parameters to what your own specification

## <h2 id = "4">Build the node

####<h4 id = "4.1">1. Setup basic workspace
In this instruction we use ~/dev/src/github.com/elastos as our working directory. If you clone the source code to a different directory, please make sure you change other environment variables accordingly (not recommended).

```shell
$ mkdir -p ~/dev/bin
$ mkdir -p ~/dev/src/github.com/elastos/
```

####<h4 id = "4.2">2. Set correct environment variables
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


####<h4 id = "4.3">3. Check Go version and glide version

Check the golang and glider version. Make sure they are the following version number or above.

```shell
$ go version
go version go1.9.2 darwin/amd64

$ glide --version
glide version 0.13.1
```

####<h4 id = "4.4">4. Clone source code to $GOPATH/src/github.com/elastos folder

Make sure you are in the folder of $GOPATH/src/github.com/elastos folder

```shell
$ git clone https://github.com/elastos/Elastos.ELA.SideChain.NeoVM.git

```

If clone works successfully, you should see folder structure like $GOPATH/src/github.com/elastos/Elastos.ELA.SideChain.NeoVM/Makefile

####<h4 id = "4.5"> 5. Install dependencies using Glide

```shell
$ cd $GOPATH/src/github.com/elastos/Elastos.ELA.SideChain.NeoVM
$ glide update && glide install
```
if the terminal report "Cannot detect VCS", we need to set mirror, please search it on web.

####<h4 id = "4.6"> 6. Make

Build the node

```shell
$ cd $GOPATH/src/github.com/elastos/Elastos.ELA.SideChain.NeoVM
$ make
```

If you did not see any error message, congratulations, you have made the NeoVM full node.

####<h4 id = "4.7"> 7. Run the node

```shell
$ ./sideNeo
```

##<h2 id = "5"> Build and Run using docker

Alternatively, if don't want to build it manually on Mac or Linux, we also provide a `Dockerfile` to help you (You need to have docker installed).

####<h4 id = "5.1">  1. Build the docker node

```shell
$ cd docker
$ docker build -t ela_node_run .
```

####<h4 id = "5.2"> 2. Run the node in the docker container

```shell
$ docker run -p 20334:20334 -p 20335:20335 -p 20336:20336 -p 20338:20338 ela_node_run
```

> Note: Don't hit Ctrl-C to terminate the output; instead close this terminal and open another.

> Please note the dockerfile uses the default 'config.json' in the repository. If you're familiar with docker, you can change the docker file to make it use your own ELA Node configuration file.


##<h2 id = "6"> License

This project is licensed under the terms of the [MIT license](https://github.com/elastos/Elastos.ELA/blob/master/LICENSE).