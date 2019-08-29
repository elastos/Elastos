# Elastos SPV C++

- [Summary](#summary)
- [Build Guide](#build-guide)
	- [Prepare](#prepare-source-code)
	- [Build on Ubuntu/MacOS](#build-on-ubuntumacos)
	- [Build for IOS](#build-for-ios)
	- [Build for Android](#build-for-android)
- [Interface Document](#interface-document)
- [Development](#development)
- [More](#more)

## Summary

This repository is a basic library aimed to provide a serials of wallet related interfaces for anyone who want to build a wallet themselves.

## Build Guide

Make sure your computer have installed the required packages below:

- [git](https://www.git-scm.com/downloads)
- [cmake](https://cmake.org/download)
- [wget](https://www.gnu.org/software/wget)
- [xcode](https://developer.apple.com/xcode/download)  (for IOS or MacOS)
- [ndk](https://developer.android.com/ndk/downloads/)  (for Android)

### Prepare Source Code
```shell
$ cd /home/xxx/dev/
$ git clone git@github.com:elastos/Elastos.ELA.SPV.Cpp.git
$ cd Elastos.ELA.SPV.Cpp
$ git submodule init
$ git submodule update --force --recursive
```

### Build on Ubuntu/MacOS
For MacOS, you need to run command below to use toolchain of xcode
```shell
$ sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
$ xcrun -find clang
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
```

And then, build
```shell
$ cd /home/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build
$ cd cmake-build
$ cmake ..
$ make -j 8
```

### Build for IOS
1. Architecture armv7 and arm64.

   ```shell
    $ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
    $ mkdir cmake-build-ios
    $ cd cmake-build-ios
    $ cmake -DSPV_PLATFORM=IOS -DCMAKE_INSTALL_PREFIX=$SOMEWHERE/spvsdk/ios/arm ..
    $ make -j 8 && make install
   ```

2. Simulator

   x86

   ```shell
   $ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
   $ mkdir cmake-build-simulator
   $ cd cmake-build-simulator
   $ cmake -DSPV_PLATFORM=IOS -DIOS_PLATFORM=SIMULATOR ..
   $ make -j 8 && make install
   ```
   x86_64
   ```shell
   $ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
   $ mkdir cmake-build-simulator64
   $ cd cmake-build-simulator64
   $ cmake -DSPV_PLATFORM=IOS -DIOS_PLATFORM=SIMULATOR64 ..
   $ make -j 8 && make install
   ```

Note: If built successfully, you will see output static library in directory *cmake-build-ios/lib/* or in installed directory, which combined with all dependent static libraries(libsqlite.a libboost_*.a libcrypto.a libssl.a libbigint.a). Support minimum IOS target version is 10.0

### Build for Android

`NDK` version: r16+

Unzip to somewhere, for example */Users/xxx/dev/android-ndk-r16*

Set system environment variable **ANDROID_NDK** to */Users/xxx/dev/android-ndk-r16*

Support architecture **arm64-v8a** **armeabi-v7a** and simulator( **x86** **x86_64** )

Set **CMAKE_ANDROID_ARCH_ABI** properly to fit your need

```shell
$ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build-ndk
$ cd cmake-build-ndk
$ cmake -DSPV_PLATFORM=Android -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a ..
$ make -j 8 && make install
```
## Interface Document
```shell
$ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
$ ./doxygen.sh
```
The documents will generate in the **doc/** directory

## Development
Patches are welcome. Please submit pull requests against the **dev** branch.


## More

Learn more about this repository please refer to the following links:

- [wiki](https://github.com/elastos/Elastos.ELA.SPV.Cpp/wiki)
