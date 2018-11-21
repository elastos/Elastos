# Elastos.ELA.SPV.Cpp

## Summary
This repository is a basic library aimed to provide a serials of wallet related interfaces for anyone who want to build a wallet themselves.


## Build on Ubuntu/MacOs
### Check the required tools
Make sure your computer have installed the required packages below:
* [git](https://www.git-scm.com/downloads)
* [cmake](https://cmake.org/download)
* [wget](https://www.gnu.org/software/wget)

### Clone source code
Open a terminal, go to `/home/xxx/dev`
```shell
$ cd /home/xxx/dev/
$ git clone git@github.com:elastos/Elastos.ELA.SPV.Cpp.git
$ cd Elastos.ELA.SPV.Cpp
$ git submodule init
$ git submodule update
```

### Build Elastos.ELA.SPV.Cpp

Create a build directory `cmake-build`
```shell
$ cd /home/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build
```

Execute cmake command to generate Makefile, and make
```shell
$ cd cmake-build
$ cmake ..
$ make
```

## Build for IOS

### Check the required tools

Make sure your computer have installed the required packages below:

- [git](https://www.git-scm.com/downloads)
- [cmake](https://cmake.org/download)
- [wget](https://www.gnu.org/software/wget)
- [xcode](https://developer.apple.com/xcode/download)

### Clone source code

Open terminal, go to `/Users/xxx/dev`

```shell
$ cd /Users/xxx/dev/
$ git clone git@github.com:elastos/Elastos.ELA.SPV.Cpp.git
$ cd Elastos.ELA.SPV.Cpp
$ git submodule init
$ git submodule update
```

### Build Elastos.ELA.SPV.Cpp

Create a build directory `cmake-build-ios`

```shell
$ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build-ios
```

Execute cmake command to generate Makefile, and make

```shell
$ cd cmake-build-ios
$ cmake -DSPV_PLATFORM=IOS ..
$ make
```

Note: If built successfully, you will see output static library in directory cmake-build-ios/lib/, which combined with all dependent static libraries(libsqlite.a libboost_*.a libcrypto.a libssl.a libbigint.a). Only support architecture armv7 & arm64 for iPhone now, and minimum IOS target version is 10.0

## Build for Android

### Check the required tools
Make sure your computer have installed the required packages below:
* [git](https://www.git-scm.com/downloads)
* [cmake](https://cmake.org/download)
* [wget](https://www.gnu.org/software/wget)
* [ndk](https://developer.android.com/ndk/downloads/)

### Install ndk
`NDK` version: r16+

Unzip to somewhere, for example `/Users/xxx/dev/android-ndk-r16`

Set system environment variable `ANDROID_NDK` to `/Users/xxx/dev/android-ndk-r16`

### Clone source code
Open terminal, go to `/Users/xxx/dev`
```shell
$ cd /Users/xxx/dev/
$ git clone git@github.com:elastos/Elastos.ELA.SPV.Cpp.git
$ cd Elastos.ELA.SPV.Cpp
$ git submodule init
$ git submodule update
```

### Build Elastos.ELA.SPV.Cpp

Create a build directory `cmake-build-ndk`
```shell
$ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build-ndk
```

Execute cmake command to generate Makefile, and make
```shell
$ cd cmake-build-ndk
$ cmake -DSPV_PLATFORM=Android ..
$ make
```


## Development
Patches are welcome. Please submit pull requests against the `dev` branch.


## More

Learn more about this repository please refer to the following links:
- [interfaces document](https://raindust.github.io/Elastos.ELA.SPV.Cpp.Document/)
- [wiki](https://github.com/elastos/Elastos.ELA.SPV.Cpp/wiki)

