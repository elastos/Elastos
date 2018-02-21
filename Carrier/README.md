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
$ cd /tmp/flatcc-0.5.0/build/install && sudo make install
```

To clone the repository in your environment:
```shell
$ cd ~
$ git clone --recurse-submodules https://github.com/elastos/Elastos.NET.Carrier.Native.SDK.git
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

After the Docker image build finish, then tag the new image:

```shell
$ docker tag THE-NEW-GENERATED-IMAGE-ID elastos-dev
```

#### 2. Start Docker image

Then you can start the Docker image:

```shell
$ docker run -tiv $(SRC_ROOT):/home/elastos/Projects --tmpfs=/tmp elastos-dev /bin/bash
```

#### 3. Build in docker

```shell
$ cd ~/Projects/build
$ ./linux_build.sh
```

## Cross-compilation

### Android

You need to get android NDK packages to build carrier NDKs for several targets, and of which will be built into Android Java carrier SDK.

#### Build on Linux

##### 1. Prepare enviroment

Download android NDK package for Linux (r13b or higher), and unzip it to $YOUR-PATH/TO.
Latest Stable Version (r16b): https://dl.google.com/android/repository/android-ndk-r16b-linux-x86_64.zip

Add the following command to ${HOME}/.bashrc to setup $ANDROID_NDK_HOME environment.

```shell
export ANDROID_NDK_HOME=YOUR-PATH/TO/android-ndk-r16b
```

Then run the command to make effect.

```shell
source ${HOME}/.bashrc
```

##### 2. Cross-compilation build

Run the build script with wanted target name under ${SRC_ROOT}/build. For example, the following command is to build Carrier NDK for armv7 target.

```shell
./android_build.sh arm
```

Currently, you can 'cross-'build Carrier NDKs for arm, arm64, x86 and x86_64 targets.

For more build options, run build script with "help" option.

```shell
./android_build.sh help
```

#### Build on MacOS

You also can build android Carrier NDKs on MacOS with the same steps on Linux except for using android NDKs packages for Darwin.

### iOS

You shold build iOS Carrier NDKs on MacOS with xCode supported.

Run the following command to build Carrier NDK for arm64:

```
./ios_build.sh arm64
```

or, use following command:

```
./ios_build.sh x86_64
```

to build Carrier NDK for x86_64 target.

For more build options, run build script with "help" option.

```
./ios_build.sh help
```

### RaspberryPi

#### 1. Prepare environment

The cross-compilation for RaspberryPi should be done on Linux platform. You need to download raspberry toolchains from https://github.com/raspberrypi/tools to $YOUR-PATH/TO. then, add the following command to ${HOME}/.bashrc to setup RASPBERRY_TOOLCHAIN_HOME environment.

```shell
export RASPBERRY_TOOLCHAIN_HOME=YOUR-PATH/TO/arm-bcm2708
```

Then run the command to make effect.

```shell
source ${HOME}/.bashrc
```

##### 2. Cross-compilation build

Run the build script with 'armv7l' option to build Carrier NDK for raspberry target.

```shell
./linux_build.sh armv7l
```

For more build options, run build script with "help" option.

```shell
./linux_build.sh armv7l help
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
$ sudo python /tmp/get-pip.py
$ sudo pip install breathe
```

#### 2. Build

Change to `$(SRC_ROOT)/docs` directory, run:

```shell
$ make html
```
