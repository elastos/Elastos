# Elastos.ELA [![Build Status](https://travis-ci.org/elastos/Elastos.ELA.svg?branch=master)](https://travis-ci.org/elastos/Elastos.ELA)

## Summary

Elacoin is the digital currency solution within Elastos eco system.

This project is the source code that can build a full node of ELA.

## Build on Mac

### Check OS version

Make sure the OSX version is 16.7+

```shell
$ uname -srm
Darwin 16.7.0 x86_64
```

### Install Go distribution 1.9

Use Homebrew to install Golang 1.9.

```shell
$ brew install go@1.9
```

> If you install older version, such as v1.8, you may get missing math/bits package error when build.

### Setup basic workspace
In this instruction we use ~/dev/src/github.com/elastos as our working directory. If you clone the source code to a different directory, please make sure you change other environment variables accordingly (not recommended). 

```shell
$ mkdir -p ~/dev/bin
$ mkdir -p ~/dev/src/github.com/elastos/
```

### Set correct environment variables

```shell
export GOROOT=/usr/local/opt/go@1.9/libexec
export GOPATH=$HOME/dev
export GOBIN=$GOPATH/bin
export PATH=$GOROOT/bin:$PATH
export PATH=$GOBIN:$PATH
```

### Install Glide

Glide is a package manager for Golang. We use Glide to install dependent packages.

```shell
$ brew install --ignore-dependencies glide
```

### Check Go version and glide version

Check the golang and glider version. Make sure they are the following version number or above.

```shell
$ go version
go version go1.9.2 darwin/amd64

$ glide --version
glide version 0.13.1
```

If you cannot see the version number, there must be something wrong when install.

### Clone source code to $GOPATH/src/github.com/elastos folder
Make sure you are in the folder of $GOPATH/src/github.com/elastos
```shell
$ git clone https://github.com/elastos/Elastos.ELA.git
```

If clone works successfully, you should see folder structure like $GOPATH/src/github.com/elastos/Elastos.ELA/Makefile
### Glide install

cd $GOPATH/src/github.com/elastos/Elastos.ELA and Run `glide update && glide install` to install dependencies.

### Make

cd $GOPATH/src/github.com/elastos/Elastos.ELA and Run `make` to build files.

If you did not see any error message, congratulations, you have made the ELA full node.

## Run on Mac

- run `./ela` to run the node program.

## Build on Ubuntu

### Check OS version

Make sure your ubuntu version is 16.04+

```shell
$ cat /etc/issue
Ubuntu 16.04.3 LTS \n \l
```

### Install basic tools

```shell
$ sudo apt-get install -y git
```

### Install Go distribution 1.9

```shell
$ sudo apt-get install -y software-properties-common
$ sudo add-apt-repository -y ppa:gophers/archive
$ sudo apt update
$ sudo apt-get install -y golang-1.9-go
```

> If you install older version, such as v1.8, you may get missing math/bits package error when build.

### Setup basic workspace
In this instruction we use ~/dev/src/github.com/elastos/ as our working directory. If you clone the source code to a different directory, please make sure you change other environment variables accordingly (not recommended). 

```shell
$ mkdir -p ~/dev/bin
$ mkdir -p ~/dev/src/github.com/elastos
```

### Set correct environment variables

```shell
export GOROOT=/usr/lib/go-1.9
export GOPATH=$HOME/dev
export GOBIN=$GOPATH/bin
export PATH=$GOROOT/bin:$PATH
export PATH=$GOBIN:$PATH
```

### Install Glide

Glide is a package manager for Golang. We use Glide to install dependent packages.

```shell
$ cd ~/dev
$ curl https://glide.sh/get | sh
```

### Check Go version and glide version

Check the golang and glider version. Make sure they are the following version number or above.

```shell
$ go version
go version go1.9.2 linux/amd64

$ glide --version
glide version v0.13.1
```

If you cannot see the version number, there must be something wrong when install.

### Clone source code to $GOPATH/src/github.com/elastos folder
Make sure you are in the folder of $GOPATH/src/github.com/elastos
```shell
$ git clone https://github.com/elastos/Elastos.ELA.git
```

If clone works successfully, you should see folder structure like $GOPATH/src/github.com/elastos/Elastos.ELA/Makefile
### Glide install

cd $GOPATH/src/github.com/elastos/Elastos.ELA and Run `glide update && glide install` to install dependencies.

### Make

cd $GOPATH/src/github.com/elastos/Elastos.ELA and Run `make` to build files.

If you did not see any error message, congratulations, you have made the ELA full node.

## Run on Ubuntu

- run `./ela` to run the node program.

# Config the node

See the [documentation](./docs/config.json.md) about config.json

## Bootstrap using docker

Alternatively if don't want to build it manually. We also provide a `Dockerfile` to help you (You need have a prepared docker env).

```bash
cd docker
docker build -t ela_node_run .

#start container

docker run -p 20334:20334 -p 20335:20335 -p 20336:20336 -p 20338:20338 ela_node_run
```

> Note: don't using Ctrl-C to terminate the output, just close this terminal and open another.

Now you can access ELA Node's rest api:

```bash
curl http://localhost:20334/api/v1/block/height
```

In the above instruction, we use default configuration file `config.json` in the repository; If you familiar with docker you can change the docker file to use your own ELA Node configuration.

## More

If you want to learn the API of Elastos.ELA, please refer to the following:

- [Restful_API](docs/Restful_API.md)
