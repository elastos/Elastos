# Elastos.ELA

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
In this instruction we use ~/dev/src as our working directory. If you clone the source code to a different directory, please make sure you change other environment variables accordingly (not recommended). 

```shell
$ mkdir ~/dev/bin
$ mkdir ~/dev/src
```

### Set correct environment variables.

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

### Clone source code to $GOPATH/src folder
Make sure you are in the folder of $GOPATH/src
```shell
$ git clone https://github.com/elastos/Elastos.ELA.git
```

If clone works successfully, you should see folder structure like $GOPATH/src/Elastos.ELA/makefile
### Glide install

Run `glide update && glide install` to install depandencies.

### Make

Run `make` to build files.

If you did not see any error message, congratulations, you have made the ELA full node. 
## Run on Mac

- run ./node to run the node program.

## Build on Ubuntu

### Check OS version

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
In this instruction we use ~/dev/src as our working directory. If you clone the source code to a different directory, please make sure you change other environment variables accordingly (not recommended). 

```shell
$ mkdir ~/dev/bin
$ mkdir ~/dev/src
```

### Set correct environment variables.

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

### Clone source code to $GOPATH/src folder
Make sure you are in the folder of $GOPATH/src
```shell
$ git clone https://github.com/elastos/Elastos.ELA.git
```

If clone works successfully, you should see folder structure like $GOPATH/src/Elastos.ELA/makefile
### Glide install

Run `glide update && glide install` to install depandencies.

### Make

Run `make` to build files.

If you did not see any error message, congratulations, you have made the ELA full node. 
## Run on Ubuntu

- run ./node to run the node program.
