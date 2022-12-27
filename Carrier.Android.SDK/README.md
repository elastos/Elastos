Elastos Carrier Android SDK
===========================

[![Build Status](https://travis-ci.com/elastos/Elastos.NET.Carrier.Android.SDK.svg)](https://travis-ci.com/elastos/Elastos.NET.Carrier.Android.SDK)

## Introduction

Elastos Carrier Android SDK is a Java API wrapper for the C language based [Elastos Carrier Native SDK](https://github.com/elastos/Elastos.NET.Carrier.Native.SDK). With the Android Carrier SDK, it is possible to build applications for mobile phones, tablets, wearables, TVs and car media systems that run on the Android Operating System (OS) while utilizing the functionalities of the Elastos Carrier.

The Elastos Carrier is a decentralized and encrypted peer-to-peer (P2P) communication framework that routes network traffic between virtual machines and Decentralized Applications (DApps).

The authentication process of peer nodes utilizes the Elastos Decentralized ID (DID) sidechain. （TODO）

## Build from source

### 1. Cross-compilation for Android Platform on Ubuntu / Debian / Linux or MacOS host

***************

With CMake, Elastos Carrier can be cross-compiled to run on Android as a target platform, while compilation is carried out on a(n) Ubuntu / Debian / Linux or MacOS host.

**Prerequisite**: Android NDK 'android-ndk-r16b' or higher must be downloaded onto the host.
Android NDKs (such as 'Linux 64-bit (x86)' or 'Mac' ) can be downloaded from https://developer.android.com/ndk/downloads/ .
Please make sure to extract the downloaded NDK.

Open a new terminal window.

Download the **Elastos.NET.Carrier.Native.SDK** (not the Elastos.NET.Carrier.Android.SDK) repository using Git:
```shell
$ git clone https://github.com/elastos/Elastos.NET.Carrier.Native.SDK
```

Navigate to the previously downloaded folder that contains the source code of the Carrier project.

```shell
$ cd YOUR-PATH/Elastos.NET.Carrier.Native.SDK
```

Enter the 'build' folder.
```shell
$ cd build
```

Create a new folder with the target platform name, then change directory.
```shell
$ mkdir android
$ cd android
```

To generate the required Makefile in the current directory, please make sure to first replace 'YOUR-TARGET-ARCHITECTURE'
and 'YOUR-ANDROID-NDK-HOME' with the correct option and path.

-DANDROID_ABI accepts the following target architecture options:
* armeabi-v7a
* arm64-v8a
* x86
* x86_64

Replace 'YOUR-ANDROID-NDK-HOME' with the path to the extracted NDK folder.

Run the command with the correct options described above:
```shell
$ cmake -DANDROID_ABI=YOUR-TARGET-ARCHITECTURE -DANDROID_NDK_HOME=YOUR-ANDROID-NDK-HOME -DCMAKE_TOOLCHAIN_FILE=../../cmake/AndroidToolchain.cmake ../..

```

Build the program: <br/>
Note: If "make" fails due to missing permissions, use "sudo make" instead.
```shell
$ make
```



Install the program: <br/>
Note: If "make install" fails due to missing permissions, use "sudo make install" instead.
```shell
$ make install
```


Create distribution package: <br/>
Note: If "make dist" fails due to missing permissions, use "sudo make dist" instead.
```
$ make dist
```

Note: To build for multiple target architectures separately, repeat the steps starting from:

Run the command with the correct options described above:
```shell
$ cmake -DANDROID_ABI=YOUR-TARGET-ARCHITECTURE -DANDROID_NDK_HOME=YOUR-ANDROID-NDK-HOME -DCMAKE_TOOLCHAIN_FILE=../../cmake/AndroidToolchain.cmake ../..

```

For each architecture, the distribution package will contain the following files:
```
libcrystal.so
libelacarrier.so
libelasession.so
```

These shared native libraries will have to be later imported into the Android project based on their
target architectures.

Currently, the target CPU architectures **armv7l**, **arm64**, **x86**, **x86-64** are supported.

### 2. Import Android project

**Prerequisite**: Android Studio must be installed (Download from https://developer.android.com/studio/ ).
Android NDK must be installed within Android Studio (File >> Settings >>Appearance and Behavior >> System Settings >> Android SDK -> SDK Tools (Tab) -> NDK (active))

Option 1: Start Android Studio and select "Check out project from Version Control", then select Git and add the URL
https://github.com/elastos/Elastos.NET.Carrier.Android.SDK , click clone.

Option 2: Download the Elastos.NET.Carrier.Android.SDK with Git:
```shell
$ git clone https://github.com/elastos/Elastos.NET.Carrier.Android.SDK
```
then start Android Studio and import the project with the option 'Open an existing Android Studio project'.


Select 'Create project from existing sources', click next, click finish.

Wait for all the import processes to finish.


### 3. Import Shared Native Libraries

In the Project View on the left side, navigate to the directory **"app/native-dist/lib"** .

Under the native-dist folder, create a new folder with the name of the target architecture such as armeabi-v7a, arm64-v8a,
x86, x86-64 or all of them, depending on which are relevant for your needs.

From Step 1 (Cross-compilation for Android Platform) in this documentation, copy the created native libraries ( .so files )
into the target directories.

Your project should have following directory structure:

```
app/native-dist
   |--include
       |--ela_carrier.h
       |--ela_session.h
   |--lib
       |--armeabi-v7a
          |--libcrystal.so
          |--libelacarrier.so
          |--libelasession.so
       |--arm64-v8a
          |--libcrystal.so
          |--libelacarrier.so
          |--libelasession.so
       |--x86
          |--libcrystal.so
          |--libelacarrier.so
          |--libelasession.so
       |--x86-64
          |--libcrystal.so
          |--libelacarrier.so
          |--libelasession.so
```

The files under the subdirectory **"app/native-dist/include"** are public header files and are already exported from Carrier native.

### 4. Build Carrier SDK

Depending on which native libraries for target architectures were imported previously, adjust the code in
Elastos.NET.Carrier.Android.SDK\app\build.gradle on the following lines:

C:\Users\akoss\AndroidStudioProjects\Elastos.NET.Carrier.Android.SDK\app\build.gradle
```shell
ndk {
    abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
}
```
Only those target architectures should stay for which the native libraries were imported.

Further adjustments are optional.

In Android Studio, click 'Make Project', then 'Build Project'.

### 5. Output

After building with success, the output distribution package named **org.elastos.carrier-debug(release).aar**, carrying the JAR package and JNI shared libraries to different CPU architectures, will be put under the directory:
```
app/build/outputs/aar
```
## Usage

In your project with gradle management, add following statements in module's build.gradle to import Carrier SDK:

```markdown
dependencies {
    implementation 'org.elastos:carrier:5.3.2'
}
```

While in project with maven management, add the following statements as it's dependency:
```markdown
<dependency>
  <groupId>org.elastos</groupId>
  <artifactId>carrier</artifactId>
  <version>5.3.2</version>
  <type>pom</type>
</dependency>
```

## Basic Tests

All basic tests are located under the directory **"app/src/androidTest"**. These tests can be run in Android Studio.
Before running the tests, uncomment the **"service"** configuration in AndroidMinifest.xml.

## Build Docs

Open **Tools** tab in Android Studio and click the **Generate JavaDoc...** item to generate the Java API document.

## Contribution

We welcome contributions to the Elastos Carrier Android SDK Project.

## Acknowledgments

A sincere thank you to all teams and projects that we rely on directly or indirectly.

## License
This project is licensed under the terms of the [GPLv3 license](https://github.com/elastos/Elastos.NET.Carrier.Android.SDK/blob/master/LICENSE)

