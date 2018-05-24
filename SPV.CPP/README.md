# Elastos.ELA.SPV.Cpp

## Summary
SPV SDK and wallet.

## Build on Ubuntu
### Check OS version
Make sure your ubuntu version is 16.04+
```shell
$ cat /etc/issue
Ubuntu 16.04.3 LTS \n \l
```

### Install git
```shell
$ sudo apt-get install -y git
```

### Install cmake
```shell
$ sudo apt-get install -y cmake
```

### Install ndk
`Ndk` version: r16+

Download [ndk](https://developer.android.com/ndk/downloads/)

Unzip to somewhere, for example `/home/xxx/dev/android-ndk-r16`

Set system environment variable `ANDROID_NDK` to `/home/xxx/dev/android-ndk-r16`

### Clone source code
Open terminal, go to `/home/xxx/dev`
```shell
$ cd /home/xxx/dev/
$ git clone git@github.com:elastos/Elastos.ELA.SPV.Cpp.git
$ cd Elastos.ELA.SPV.Cpp
$ git submodule init
$ git submodule update
```

### Build Elastos.ELA.SPV.Cpp

Create a build directory `cmake-build-ndk-debug`
```shell
$ cd /home/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build-ndk-debug
```

Execute cmake command to generate Makefile, and make
```shell
$ cd cmake-build-ndk-debug
$ cmake -DSPV_FOR_ANDROID=ON -DCMAKE_BUILD_TYPE=Debug ..
$ make
```


## Build on Mac
### Check OS version
Make sure the OSX version is 16.7+

```shell
$ uname -srm
Darwin 16.7.0 x86_64
```

### Install git
Download and install [git](https://www.git-scm.com/downloads).

### Install cmake
`cmake` version: 3.11+

Download and install [cmake](https://cmake.org/download/)

### Install ndk
`NDK` version: r16+

Download [ndk](https://developer.android.com/ndk/downloads/)

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

Create a build directory `cmake-build-ndk-debug`
```shell
$ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build-ndk-debug
```

Execute cmake command to generate Makefile, and make
```shell
$ cd cmake-build-ndk-debug
$ cmake -DSPV_FOR_ANDROID=ON -DCMAKE_BUILD_TYPE=Debug ..
$ make
```


## Build on windows

Good luck.
