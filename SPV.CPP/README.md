# Elastos.ELA.SPV.Cpp

## Summary
SPV SDK and wallet.

## Build on windows

### Check OS version
The windows version should be win10+ (tested on win10)

### Install git
Download and install [git](https://www.git-scm.com/downloads).

### Install make for windows
`make` version: 3.81+

Download [make for windows](http://gnuwin32.sourceforge.net/packages/make.htm).

Select `Binaries` and `Dependencies` zip packages, and unzip them to the directory where git installed (eg: `C:\Program Files\Git\usr`)

### Install cmake
`cmake` version: 3.11+

Download and install [cmake](https://cmake.org/download/)

_Remember to select 'Add CMake to the system PATH for all users' while installing_

### Install ndk
`Ndk` version: r17+

Download [ndk](https://developer.android.com/ndk/downloads/)

Unzip to somewhere, for example `D:\dev\android-ndk-r17`

Set system environment variable `ANDROID_NDK` to `D:\dev\android-ndk-r17`

### Clone source code
Open Git Bash terminal, go to `D:\dev`
```shell
$ cd /d/dev/
$ git clone git@github.com:elastos/Elastos.ELA.SPV.Cpp.git
$ cd Elastos.ELA.SPV.Cpp
$ git submodule init
$ git submodule update
$ cd ThirdParty/Boost-for-Android
$ git remote add tmp git@github.com:heropan/Boost-for-Android.git
$ git fetch tmp
$ git reset --hard tmp/master 
```
### Build Boost-for-Android
Make sure the system environtment variable takes effect, if not, open a new git bash after `ANDROID_NDK` was set.
```shell
$ echo $ANDROID_NDK
D:\android\android-ndk-r17
```
Go to directory `D:\dev\Elastos.ELA.SPV.Cpp\ThirdParty\Boost-for-Android` and start to build
```shell
$ cd /d/dev/Elastos.ELA.SPV.Cpp/ThirdParty/Boost-for-Android
$ ./build-android.sh $ANDROID_NDK
```
Wait and have a cup of tea. ;-)

_note: if error `'cl' is not recognized as an internal or external command` occur, make sure you have visual studio installed correctly._ 
### Build Elastos.ELA.SPV.Cpp

Create a build directory `cmake-build-ndk`
```shell
$ cd /d/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build-ndk-debug
```

Execute cmake command to generate Makefile, and make
```shell
$ cd cmake-build-ndk-debug
$ cmake -G "Unix Makefiles" -DSPV_FOR_ANDROID=ON -DCMAKE_BUILD_TYPE=Debug ..
$ make
```
