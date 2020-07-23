# Elastos DID Native SDK

|Linux && Mac|Windows|
|:-:|:-:|
|[![Build Status](https://github.com/elastos/Elastos.DID.Native.SDK/workflows/CI/badge.svg)](https://github.com/elastos/Elastos.DID.Native.SDK/actions)| |

## Introduction

**Elastos DID (Decentralized Identifier) framework** is a set of C APIs for Elastos DID that is compatibility to W3C DIDs specs.

DID (Decentralized identifier) is a new type of identifier that enables verifiable, decentralized digital identity. A DID identifies any subject (e.g., a person, organization, thing, data model, abstract entity, etc.) that the controller of the DID decides that it identifies.

## Table of Contents

- [Elastos DID Native SDK](#elastos-DID-native-sdk)
  - [Introduction](#introduction)
  - [Table of Contents](#table-of-contents)
  - [Usage](#usage)
  - [Build on Ubuntu / Debian / Linux Host](#build-on-ubuntu--linux-host)
    - [1. Brief introduction](#1-brief-introduction)
    - [2. Install Pre-Requirements](#2-install-pre-requirements)
    - [3. Build to run on host (Ubuntu / Debian / Linux)](#3-build-to-run-on-host-ubuntu--debian--linux)
    - [4. Run DIDTest](#4-run-didtest)
      - [4.1. Normal Test](#5-normal-test)
      - [4.2. Stress Test](#5-stress-test)
      - [4.3. Dummy Test](#5-dummy-test)
    - [5. Cross-compilation for Android Platform](#5-cross-compilation-for-android-platform)
  - [Build on MacOS Host](#build-on-macos-host)
    - [1. Brief introduction](#1-brief-introduction-2)
    - [2. Install Pre-Requirements](#2-install-pre-requirements-2)
    - [3. Build to run on host](#3-build-to-run-on-host-1)
    - [4. Run DIDTest](#4-run-didtest-2)
      - [4.1. Normal Test](#5-normal-test)
      - [4.2. Stress Test](#5-stress-test)
      - [4.3. Dummy Test](#5-dummy-test)
    - [5. Cross-compilation for Android Platform](#5-cross-compilation-for-android-platform-1)
    - [6. Cross-compilation for iOS Platform](#6-cross-compilation-for-ios-platform)
  - [Build API Documentation](#build-api-documentation)
    - [Build on Ubuntu / Debian / Linux Host](#build-on-ubuntu--debian--linux-host-1)
      - [1. Install Pre-Requirements](#1-install-pre-requirements)
      - [2. Build](#2-build)
      - [3. View](#3-view)
  - [Contribution](#contribution)
  - [Acknowledgments](#acknowledgments)
  - [License](#license)

## Usage

**CMake** is used to build, test and package the Elastos DID project in an operating system as well as compiler-independent manner.

Certain knowledge of CMake is required.

At the time of this writing, The compilation of sources works on **macOS**, **Linux** (Ubuntu, Debian etc.) and **Windows**(support later), and provides the option to cross-compile for target systems of **iOS**, **Android** and **RaspberryPi**（support later）.

## Build on Ubuntu / Debian / Linux Host

### 1. Brief introduction

On Ubuntu / Debian / Linux, besides the compilation for the host itself, cross-compilation is possible for the following targets:

- Android with architectures of **armv7a**, **arm64** and simulators of **x86/x86_64** are supported.
- RaspberryPi with architecture **armv7l** only(support later).

### 2. Install Pre-Requirements

To generate Makefiles by using **configure** or **cmake** and manage dependencies of the DID project, certain packages must be installed on the host before compilation.

Run the following commands to install the prerequisite utilities:

```shell
sudo apt-get update
sudo apt-get install -f build-essential autoconf automake autopoint libtool flex bison libncurses5-dev cmake
```

Download this repository using Git:

```shell
git clone https://github.com/elastos/Elastos.DID.Native.SDK
```

### 3. Build to run on host (Ubuntu / Debian / Linux)

To compile the project from source code for the target to run on Ubuntu / Debian / Linux, carry out the following steps:

Open a new terminal window.

Navigate to the previously downloaded folder that contains the source code of the DID project.

```shell
cd YOUR-PATH/Elastos.DID.Native.SDK
```

Enter the 'build' folder.

```shell
cd build
```

Create a new folder with the target platform name, then change directory.

```shell
mkdir linux
cd linux
```

Generate the Makefile in the current directory:

*Note: Please see custom options below.*

```shell
cmake ../..
```

***

Optional (Generate the Makefile): To be able to build a distribution with a specific build type **Debug/Release**, as well as with customized install location of distributions, run the following commands:

```shell
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=YOUR-INSTALL-PATH ../..
```

***

Build the program:

*Note: If "make" fails due to missing permissions, use "sudo make" instead.*

```shell
make
```

Install the program:

*Note: If "make install" fails due to missing permissions, use "sudo make install" instead.*

```shell
make install
```

Create distribution package:

*Note: If "make dist" fails due to missing permissions, use "sudo make dist" instead.*

```shell
make dist
```

### 4. Run DIDTest

DIDTest is a shell program to imitate every DID flow and to prove DID API . The output is displayed in the terminal for a simple evaluation of test results.

DIDTest supports three modules: normal test, stress test and dummy test.

To run DIDTest, first extract the distribution package created previously and enter the extracted folder. Then, change directory to the 'bin' folder.

```shell
cd YOUR-DISTRIBUTION-PACKAGE-PATH/bin
```

#### 4.1 Normal Test

Run DIDTest as normal module, including dummy test cases and IDChain Transaction test case. Run normal module, test case needs spv wallet to pay coins for IDChain Transaction. So the first thing is creating spv wallet, detail operation is as follow:

```shell
$ ./wallet -d YOUR-WALLET-DATA-DIRECTORY -n NETWOR-NAME
Wallet data directory: YOUR-WALLET-DATA-DIRECTORY
-> wallet $ create YOUR-WALLET-NAME
```

'YOUR-WALLET-DATA-DIRECTORY' : wallet data directory with given path;

'YOUR-WALLET-NAME' : a new wallet with given name.

After inputing 'create' command, you also need choose the mnemonic language, mnemonic word count, passphrase for encoring mnemonic and payment password. After finishing a
series of operation, you has a new spv wallet.

Second, open your test case folder:

```shell
cd YOUR-PATH/Elastos.DID.Native.SDK/tests
```

Modify 'walletdir', 'walletId' and 'walletpass' in constant.c file with your wallet parameter.

Third, rebuild the DID project as above operation. (make && make install)

Finally, Then, change directory to the 'bin' folder.

```shell
cd YOUR-DISTRIBUTION-PACKAGE-PATH/bin
./didtest
```

#### 4.2 Stress Test

At the same time, DIDTest support stress test. Use Available commands in the shell can be listed by using the command **help**. Specific command usage descriptions can be displayed by using **help [Command]** where [Command] must be replaced with the specific command name.

For example:

```shell
./didtest -s 100 -m memcheck
```

#### 4.3 Dummy Test

Run DIDTest without IDChain Transaction, this module does not need spv wallet. If you no spv wallet or only want to run basic DID functions, you can choose this module.

```shell
./didtest --dummy
```

### 5. Cross-compilation for Android Platform

COMING SOON

### 6. Cross-compilation for Raspberry Pi

COMING SOON

## Build on Raspberry Pi（support later)

COMING SOON

## Build on MacOS Host

### 1. Brief introduction

On macOS, besides the compilation for the host itself, cross-compilation is possible for the following targets:

- Android with architectures of **armv7a**, **arm64** and simulators of **x86/x86_64** are supported.
- iOS platforms to run on **iPhone-arm64** and **iPhoneSimulator-x86_64**.

### 2. Install Pre-Requirements

packages must be installed on the host before compilation.

The following packages related to **configure** and **cmake** must be installed on the host before compilation either by installation through the package manager **homebrew** or by building from source:

Note: Homebrew can be downloaded from [Homebrew web site](https://brew.sh/).

Install packages with Homebrew:

```shell
brew install autoconf automake libtool shtool pkg-config gettext cmake
```

Please note that **homebrew** has an issue with linking **gettext**. If you have an issue with the execution of **autopoint**, fix it by running:

```shell
brew link --force gettext
```

Download this repository using Git:

```shell
git clone https://github.com/elastos/Elastos.DID.Native.SDK
```

### 3. Build to run on host

To compile the project from source code for the target to run on MacOS, carry out the following steps:

Open a new terminal window.

Navigate to the previously downloaded folder that contains the source code of the DID project.

```shell
cd YOUR-PATH/Elastos.DID.Native.SDK
```

Enter the 'build' folder.

```shell
cd build
```

Create a new folder with the target platform name, then change directory.

```shell
mkdir macos
cd macos
```

Generate the Makefile in the current directory:

*Note: Please see custom options below.*

```shell
cmake ../..
```

***

Optional (Generate the Makefile): To be able to build a distribution with a specific build type **Debug/Release**, as well as with customized install location of distributions, run the following commands:

```shell
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=YOUR-INSTALL-PATH ../..
```

***

Build the program:

*Note: If "make" fails due to missing permissions, use "sudo make" instead.*

```shell
make
```

Install the program:

*Note: If "make install" fails due to missing permissions, use "sudo make install" instead.*

```shell
make install
```

Create distribution package:

*Note: If "make dist" fails due to missing permissions, use "sudo make dist" instead.*

```shell
make dist
```

### 4. Run DIDTest

DIDTest is a shell program to imitate every DID flow and to prove DID API . The output is displayed in the terminal for a simple evaluation of test results.

DIDTest supports three modules: normal test, stress test and dummy test.

To run DIDTest, first extract the distribution package created previously and enter the extracted folder. Then, change directory to the 'bin' folder.

```shell
cd YOUR-DISTRIBUTION-PACKAGE-PATH/bin
```

#### 4.1 Normal Test

Run DIDTest as normal module, including dummy test cases and IDChain Transaction test case. Run normal module, test case needs spv wallet to pay coins for IDChain Transaction. So the first thing is creating spv wallet, detail operation is as follow:

```shell
$ ./wallet -d YOUR-WALLET-DATA-DIRECTORY -n NETWOR-NAME
Wallet data directory: YOUR-WALLET-DATA-DIRECTORY
-> wallet $ create YOUR-WALLET-NAME
```

'YOUR-WALLET-DATA-DIRECTORY' : wallet data directory with given path;

'YOUR-WALLET-NAME' : a new wallet with given name.

After inputing 'create' command, you also need choose the mnemonic language, mnemonic word count, passphrase for encoring mnemonic and payment password. After finishing a
series of operation, you has a new spv wallet.

Second, open your test case folder:

```shell
cd YOUR-PATH/Elastos.DID.Native.SDK/tests
```

Modify 'walletdir', 'walletId' and 'walletpass' in constant.c file with your wallet parameter.

Third, rebuild the DID project as above operation. (make && make install)

Finally, Then, change directory to the 'bin' folder.

```shell
cd YOUR-DISTRIBUTION-PACKAGE-PATH/bin
./didtest
```

#### 4.2 Stress Test

At the same time, DIDTest support stress test. Use Available commands in the shell can be listed by using the command **help**. Specific command usage descriptions can be displayed by using **help [Command]** where [Command] must be replaced with the specific command name.

For example:

```shell
./didtest -s 100 -m memcheck
```

#### 4.3 Dummy Test

Run DIDTest without IDChain Transaction, this module does not need spv wallet. If you no spv wallet or only want to run basic DID functions, you can choose this module.

```shell
./didtest --dummy
```

### 5. Cross-compilation for Android Platform

COMING SOON

### 6. Cross-compilation for iOS Platform

With CMake, Elastos DID can be cross-compiled to run on iOS as a target platform, while compilation is carried out on a MacOS host with XCode.

**Prerequisite**: MacOS version must be **9.0** or higher.

Open a new terminal window.

Navigate to the previously downloaded folder that contains the source code of the DID project.

```shell
cd YOUR-PATH/Elastos.DID.Native.SDK
```

Enter the 'build' folder.

```shell
cd build
```

Create a new folder with the target platform name, then change directory.

```shell
mkdir ios
cd ios
```

To generate the required Makefile in the current directory, please make sure to first replace 'YOUR-IOS-PLATFORM' with the correct option.

-DIOS_PLATFORM accepts the following target architecture options:

- iphoneos
- iphonesimulator

Replace 'YOUR-IOS-PLATFORM' with the path to the extracted NDK folder.

Run the command with the correct options described above:

```shell
cmake -DIOS_PLATFORM=YOUR-IOS-PLATFORM -DCMAKE_TOOLCHAIN_FILE=../../cmake/iOSToolchain.cmake ../..

```

Build the program:

*Note: If "make" fails due to missing permissions, use "sudo make" instead.*

```shell
make
```

Install the program:

*Note: If "make install" fails due to missing permissions, use "sudo make install" instead.*

```shell
make install
```

Create distribution package:

*Note: If "make dist" fails due to missing permissions, use "sudo make dist" instead.*

```shell
make dist
```

## Build on Windows Host

COMING SOON

## Build API Documentation

Currently, the API documentation can only be built on **Linux** hosts. MacOS has a bug issue with python, which would cause build process failure.

### Build on Ubuntu / Debian / Linux Host

#### 1. Install Pre-Requirements

```shell
sudo apt-get update
sudo apt-get install doxygen python-sphinx graphviz
curl -L -o /tmp/get-pip.py https://bootstrap.pypa.io/get-pip.py
sudo python /tmp/get-pip.py
sudo pip install breathe
```

#### 2. Build

Change to the directory where the build for any target has been previously executed. For example, if the target was Linux, the folder structure would be similar to:

```shell
cd /YOUR-PATH/Elastos.DID.Native.SDK/build/linux
```

Run the following command:

*Note: If "make" fails due to missing permissions, use "sudo make" instead.*

```shell
cmake -DENABLE_DOCS=ON ../..
make
```

#### 3. View

The generated API documentation will be created in the new **/docs** directory on the same directory level.

Change to the docs folder:

```shell
cd docs/html
```

Open the index.html file in a browser from the terminal:

```shell
xdg-open index.html
```

## Contribution

We welcome contributions to the Elastos DID Project.

## Acknowledgments

A sincere thank you to all teams and projects that we rely on directly or indirectly.

## License

This project is licensed under the terms of the [MIT license](https://github.com/elastos/Elastos.DID.Native.SDK/blob/master/LICENSE).
