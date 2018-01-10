# Elastos Carrier Native SDK

## Summary

Elastos Carrier is a decentralized peer to peer communication framework.

## Build from source

### Ubuntu

#### 1. Install Pre-Requirements

Execute following command to install all pre-requirements.

```shell
$ sudo apt-get update
$ sudo apt-get install build-essential autoconf automake autopoint libtool flex bison libncurses5-dev
$ curl -L -o /tmp/flatcc-0.5.0.tar.gz https://github.com/dvidelabs/flatcc/archive/v0.5.0.tar.gz
$ cd /tmp && tar xzvf flatcc-0.5.0.tar.gz
$ mkdir -p /tmp/flatcc-0.5.0/build/install
$ cd /tmp/flatcc-0.5.0/build/install && cmake ../.. -DFLATCC_INSTALL=on
$ cd /tmp/flatcc-0.5.0/build/install && make install
```

#### 2. Build

Change to `$(SRC_ROOT)/build` directory, and run:

```shell
$ ./linux_build.sh
```

For more build options, run build script with "help" option.

```shell
$ ./linux_build.sh help
```

### MacOS

#### 1. Install Pre-Requirements

You need to install the following packages on your Mac to build from the source:

```shell
autoconf automake libtool shtool pkg-config gettext
```

You can use brew or build the packages from the source code.

#### 2. Build

Change to `$(SRC_ROOT)/build` directory, and run:

```shell
$ ./darwin_build.sh
```

For more build options, run build script with "help" option.

```shell
$ ./darwin_build.sh help
```

### Using pre-configured Docker image

#### 1. Prepare the Docker image

Change to `$(SRC_ROOT)`/docker directory, and run:

```shell
$ docker build .
```

After the Docker image build finish, the tag the new image:

```shell
$ docker tag THE-NEW-GENERATED-IMAGE-ID elastos-dev
```

#### 2. Start Docker image

Then you can start the Docker image:

```shell
$ docker run -tiv $(SRC_ROOT):/home/elastos/Projects --tmpfs=/tmp elastos-dev /bin/bash
```

3. Build in docker

```shell
$ cd ~/Projects/build
$ ./linux_build.sh
```

## Test

After success finished build from the source code, change directory to `$(SRC_ROOT)/apps/shell`.
This demo application is an interactive shell for Elastos Carrier. Run following command to start
the shell.

```shell
$ ./elashell.sh
```

## Build API documentation

The documentation can only build on the Linux host. The python on the MacOS has a critical bug 
will cause the build process failed.

### Ubuntu

#### 1. Install Pre-Requirements

```shell
$ sudo apt-get update
$ sudo apt-get install doxygen python-sphinx graphviz
$ curl -L -o /tmp/get-pip.py https://bootstrap.pypa.io/get-pip.py
$ python /tmp/get-pip.py
$ pip install breathe
```

#### 2. Build

Change to `$(SRC_ROOT)/docs` directory, run:

```shell
$ make html
```
