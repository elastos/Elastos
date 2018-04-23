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

```
autoconf automake libtool shtool pkg-config gettext flatcc
```

You can use brew or build the packages from the source code.

Beware, homebrew has an issue with linking `gettext`. So if you having an issue with executing `autopoint`, run:

```shell
$ brew link --force gettext
```

The required flatcc version is **0.5.0**, you can install it from source code:

```shell
$ cd $FLATCC_SRC_ROOT
$ mkdir -p build/install
$ cd build/install
$ cmake ../.. -DBUILD_SHARED_LIBS=off -DFLATCC_RTONLY=off -DFLATCC_TEST=off \
        -DCMAKE_BUILD_TYPE=Release -DFLATCC_INSTALL=on
$ make
$ sudo make install
```

And run resulting command.

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
$ docker build -t elastos-dev .
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

You need to get android NDK packages to build carrier NDKs for several targets, and of which will be built into Android Java carrier SDK. **The minimum Android NDK API level required by Elastos Carrier is 21**. 

#### Build on Linux

##### 1. Prepare enviroment

Download android NDK package for Linux (r13b or higher, suggested for r16b), and unzip it to $YOUR-PATH/TO.

Add the following command to ${HOME}/.bashrc to setup $ANDROID_NDK_HOME environment.

```shell
$ export ANDROID_NDK_HOME=YOUR-PATH/TO/android-ndk-r16b
```

Then run the command to make effect.

```shell
$ source ${HOME}/.bashrc
```

##### 2. Cross-compilation build

Run the build script with wanted target name under ${SRC_ROOT}/build. For example, the following command is to build Carrier NDK for Android ARMv7 target.

```shell
$ ./android_build.sh arm
```

And the following command is to build Carrier NDK for Android ARMv8a target.

```shell
$ ./android_build.sh arm64
```

Currently, you can 'cross-'build Carrier NDKs for arm, arm64, x86 and x86_64 targets.

For more build options, run build script with "help" option.

```shell
$ ./android_build.sh help
```

#### Build on MacOS

You also can build android Carrier NDKs on MacOS with the same steps on Linux except for using android NDKs packages for Darwin.

### iOS

You shold build iOS Carrier NDKs on MacOS with xCode supported.

Run the following command to build Carrier NDK for arm64:

```shell
$ ./ios_build.sh arm64
```

or, use following command:

```shell
$ ./ios_build.sh x86_64
```

to build Carrier NDK for x86_64 target.

For more build options, run build script with "help" option.

```shell
$ ./ios_build.sh help
```

### RaspberryPi

#### 1. Prepare environment

The cross-compilation for RaspberryPi should be done on Linux platform. You need to download raspberry toolchains from https://github.com/raspberrypi/tools to $YOUR-PATH/TO. then, add the following command to ${HOME}/.bashrc to setup RASPBERRY_TOOLCHAIN_HOME environment.

```shell
$ export RASPBERRY_TOOLCHAIN_HOME=YOUR-PATH/TO/arm-bcm2708
```

Then run the command to make effect.

```shell
$ source ${HOME}/.bashrc
```

##### 2. Cross-compilation build

Run the build script with 'armv7l' option to build Carrier NDK for raspberry target.

```shell
$ ./linux_build.sh armv7l
```

For more build options, run build script with "help" option.

```shell
$ ./linux_build.sh armv7l help
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

