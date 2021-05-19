# Elastos SPV C++

- [Summary](#summary)
- [Build Guide](#build-guide)
	- [Build on Ubuntu/MacOS](#build-on-ubuntumacos)
	- [Build for IOS](#build-for-ios)
	- [Build for Android](#build-for-android)
- [Interface Document](#interface-document)
- [Development](#development)
- [More](#more)

## Summary

This repository is a basic library aimed to provide a serials of wallet related interfaces.

## Build Guide

Make sure your computer have installed the required packages below:

- [git](https://www.git-scm.com/downloads)
- [cmake](https://cmake.org/download)
- [wget](https://www.gnu.org/software/wget)
- [xcode](https://developer.apple.com/xcode/download)  (for IOS or MacOS)
- [ndk](https://developer.android.com/ndk/downloads/)  (for Android)

Download this repository using Git:
```shell
$ git clone git@github.com:elastos/Elastos.ELA.SPV.Cpp.git
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
$ cd YOUR-PATH/Elastos.ELA.SPV.Cpp
$ cd Build
$ mkdir host && cd host
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=YOUR-INSTALL-PATH ../..
$ make -j NCPU && make install
```

### Build for IOS
1. Architecture (arm64)

   ```shell
    $ cd YOUR-PATH/Elastos.ELA.SPV.Cpp
    $ cd Build
    $ mkdir iphoneos && cd iphoneos
    $ cmake -IOS_PLATFORM=iphoneos -DCMAKE_TOOLCHAIN_FILE=../../CMake/iOSToolchain.cmake -DCMAKE_INSTALL_PREFIX=YOUR-INSTALL-PATH ../..
    $ make -j NCPU && make install
   ```

2. Simulator (x86_64)

   ```shell
   $ cd YOUR-PATH/Elastos.ELA.SPV.Cpp
   $ cd Build
   $ mkdir iphonesimulator && cd iphonesimulator
   $ cmake -IOS_PLATFORM=iphonesimulator -DCMAKE_TOOLCHAIN_FILE=../../CMake/iOSToolchain.cmake -DCMAKE_INSTALL_PREFIX=YOUR-INSTALL-PATH ../..
   $ make -j NCPU && make install
   ```

### Build for Android

`NDK` version: r16+

To generate the required Makefile in the current directory, please make sure to first replace 'YOUR-TARGET-ARCHITECTURE' and 'YOUR-ANDROID-NDK-HOME' with the correct option and path.

-DANDROID_ABI accepts the following target architecture options:
- armeabi-v7a
- arm64-v8a
- x86
- x86_64

```shell
$ cd YOUR-PATH/Elastos.ELA.SPV.Cpp
$ cd Build
$ mkdir android
$ cd android
$ cmake -DANDROID_ABI=YOUR-TARGET-ARCHITECTURE -DANDROID_NDK_HOME=YOUR-ANDROID-NDK-HOME -DCMAKE_TOOLCHAIN_FILE=../../CMake/AndroidToolchain.cmake -DCMAKE_INSTALL_PREFIX=YOUR-INSTALL-PATH ../..
$ make -j NCPU && make install
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
