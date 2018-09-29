Elastos Carrier Native SDK
==========================
[![Build Status](https://travis-ci.org/elastos/Elastos.NET.Carrier.Native.SDK.svg)](https://travis-ci.org/elastos/Elastos.NET.Carrier.Native.SDK)
[![CircleCI](https://circleci.com/gh/elastos/Elastos.NET.Carrier.Native.SDK.svg?style=svg)](https://circleci.com/gh/elastos/Elastos.NET.Carrier.Native.SDK)

## Summary

Elastos Carrier is a decentralized peer to peer communication framework.

## Build from source

Elastos Carrier has been totally switched to use **CMake** to conduct the build, tests and packages of itself, and even of its internal dependencies, which make the whole compilation process become simple and easy to conduct. It would be better with certain knowledge of CMake if possible.

At the time of this writing, The compilation of sources could work on **macOS**, **Linux** (Ubuntu, Debian etc) and **Windows**, and particularly provide cross-compilation for target systems of **iOS**, **Android** and **RaspberryPi** etc.

### Ubuntu

#### 1. Brief intrudcution

On ubuntu/Debian Linux, besides from to make compilation for host itself, we also support to make cross-compilation for following targets:

* Android with architectures of **armv7a**, **arm64** and simulators of **x86/x86_64** supported.
* RespberryPi with architecture **armv7l** only.

#### 2. Install Pre-Requirements

In case that all dependency projects of Carrier, either use **configure** or **cmake** to generate Makefiles, so certain auxilary utilitiy packages must be required on host before any compilation.

Run the following commands directly to install the prerequisite utilities:

```shell
$ sudo apt-get update
$ sudo apt-get install -f build-essential autoconf automake autopoint libtool flex bison libncurses5-dev cmake
```

#### 3. Build to Run on Host

Once you have souce tree, to make compilation for the target to run on host Linux, need to execute following commands under directory of `${YOUR-SOURCE-ROOT}/build`:

```shell
$ cd YOUR-SOURCE-ROOT/build
$ mkdir linux
$ cd linux
$ cmake ../..
$ make
```
Also to build distribution with specific build type **Debug/Release**, as well as with customized install location of distributions, run the following commands:

```shell
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=YOUR-INSTALL-PATH ../..
$ make
$ make install
```
Eventually, to run the following command to release distribution package for your pre-release tests or even to customers.

```
$ make dist
```

#### 4. Cross-compilation for Android Platform

Elastos Carrier cmake system supports to build cross-compilation for android platform on Linux, which must be armed with android NDK toolchains，required with minimum API level **21**. We recommend to use **android-ndk-r16b** or higher version.

Once having android NDK installed, run the following commands to build android distributions with target **arm64** as an example:

```
$ cd YOUR-SOURCE-ROOT/build
$ mkdir android
$ cd android
$ cmake -DANDROID_ABI=arm64-v8a -DANDROID_NDK_HOME=YOUR-ANDROID-NDK-HOME -DCMAKE_TOOLCHAIN_FILE=../../cmake/AndroidToolchain.cmake ../..
$ make
$ make install
$ make dist
```
where option **ANDROID_ABI** should be one of the target architectures listed below:

* armeabi-v7a
* arm64-v8a
* x86
* x86_64

Beware, the default installation of distributions are internally designed to be **outputs** of current working directory.

#### 5. Cross-compilation for RaspberryPi

Elastos Carrier cmake system supports to build cross-compilation for RaspberryPi on Linux, which must be armed with RaspberryPi toolchains. To be noticed, currently, cross-compilation for RaspberryPi only be allowed on Linux.

As to toolchains, you are recommended to download it from the github address:

```
https://github.com/raspberrypi/tools
```
Once having RaspberryPi toolchains installed, run the following commands to build target distributions:

```
$ cd YOUR-SOURCE-ROOT/build
$ mkdir rpi
$ cd rpi
$ cmake -DRPI_TOOLCHAIN_HOME=YOUR-RASPBERRYPI-TOOLCHAIN-HOME -DCMAKE_TOOLCHAIN_FILE=../../cmake/RPiToolchain.cmake ../..
$ make
$ make install
$ make dist
```
As same to cross-compilation for android, the default installation of distributions are internally located under **outputs** of your working directory.

### macOS

#### 1. Brief intrudcution

On macOS, besides from to make compilation for maocOS itself, we also support to make cross-compilation for the following targets:

* Android with architectures for **armv7a**, **arm64** and simulators of **x86/x86_64**, as same to capabilities on Linux.
* iOS platforms to run on **iPhone-arm64** and **iPhoneSimulator-x86_64**.


#### 2. Install Pre-Requirements

As same to the reqirement on Linux, the following auxilary packages related to **configure** and **cmake** must be installed on host before any compilation, either by installation through utility **homebrew** or by building from source:

```
autoconf automake libtool shtool pkg-config gettext cmake
```

Beware, **homebrew** has an issue with linking **gettext**. So if you have an issue with execution of **autopoint**, fix it by run:

```shell
$ brew link --force gettext
```

#### 3. Build to Run on macOS

Once having source tree on macOS, run the same commands as addressed on Linux for the chapter of **build to run on host**. No extra special commands are needed for compiplation on Mac other than on Linux.

#### 4. Cross-compilation for Android Platform.

Elastos Carrier cmake system also supports to build cross-compilation for android platform on macOS with android NDK toolchains of same requirement of minimum API level **21**.

As to cross-comiplation in practice, refer to the commands as described on Linux for the chapter of **cross-compilation for android platform**. Be aware again, no extra special commands is required on macOS other than on Linux.

#### 5. Cross-compilation for iOS Platform

Elastos Carrier cmake system supports to build cross-compilation for iOS platform on macOS shipping with Apple Xcode of the minum iOS verison **9.0** supported, on top of which, run the following commands to build distributions to run on iPhone as an example:

```
$ cd YOUR-SOURCE-ROOT/build
$ mkdir ios
$ cd ios
$ cmake -DIOS_PLATFORM=iphoneos -DCMAKE_TOOLCHAIN_FILE=../../cmake/iOSToolchain.cmake ../..
$ make
$ make install
$ make dist
```
where option **IOS_PLATFORM** should be one of the target platforms listed below:

* iphoneos
* iphonesimulator

As same to run on Linux, the default installtion of distributions would be located to **outputs** under working directory unless with customized path by feeding option **CMAKE_INSTALL_PREFIX**.

### Windows

#### 1. Brief intrudcution

On Windows, Elastos Carrier cmake system only supports to compile for targets to run on Windows itself, but with 32-bits and 64-bits both supported, and visual studio IDE is mandatorily required. We recommend to use Visaul Studio IDE community 2017.

To build target for 32-bits, you need to choose `x86 Native Tools Command Console` for sure to run building commands, otherwise, run commands in `x64 Native Tools Command Console`.

#### 2. Build

Once having source tree and settling with command console, run the following commands under directory of `YOUR-SOURCE-ROOT/build`:

```
$ cd YOUR-SOURCE-ROOT/build
$ mkdir win
$ cd win
$ cmake -G "NMake Makefiles” -DCMAKE_INSTALL_PREFIX=outputs ..\..
$ nmake
$ nmake install
$ nmake dist
```
## Test

After successfully finished building from the source code, change directory to `${YOUR-INSTALL-PATH}/bin`,  and run shell demo application, which is an interactive shell for Elastos Carrier tester or beginner.

```shell
$ cd YOUR-INSTALL-PATH/bin
$ ./elashell.sh
```

You are also recommended to run api-level test suites to check whether Carrier APIs function working or not.

```
$ cd YOUR-INSTALL-PATH/bin
$ ./elatests.sh
```

## Build API Docs

Currently, the API documentation can only be built on the **Linux** host. MacOS has a bug issue with python, which would cause build process failure.

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

Run the command under the directory where runs your building commands:

```shell
$ cmake -DENABLE_DOCS=ON ../..
$ make
```

Then, you will find new directory **docs** generated, which contains all APIs documentations with **html** format.

## Thanks

Sincerely thanks to all teams and projects that we relies on directly or indirectly.

## Contributing

We welcome contributions to the Elastos Carrier Project.

## License
MIT
