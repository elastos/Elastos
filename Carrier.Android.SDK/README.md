Elastos Carrier Android SDK
===========================

[![Build Status](https://travis-ci.org/elastos/Elastos.NET.Carrier.Android.SDK.svg)](https://travis-ci.org/elastos/Elastos.NET.Carrier.Android.SDK)

## Summary

Elastos Carrier Android SDK is java api wrapper for Elastos Native Carrier, where Carrier is a decentralized peer to peer communication framework.

## Build from source

### 1.Build Carrier NDK

You need to build carrier android ndk distributions from the Carrier native repository with following github address.

```
https://github.com/elastos/Elastos.NET.Carrier.Native.SDK
```

Finished building android ndk for Carrier, you would have the following native shared libraries:

```
libcrystal.so
libelacarrier.so
libelasession.so
```

to each CPU architecture, currently supported for **armv7l**, **arm64**, **x86**, **x86-64** respectively.

### 2.Import Carrier NDK

The directory **"app/native-dist"** to import native libraries and headers should have following directory structure:

```
app/native-dist
   |--include
       |--ela_carrier.h
       |--ela_session.h
   |--libs
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

The headers under subdirectory **"include"** are public header files exported from Carrier native. And shared libraries under **libs** to each CPU arch are built from Carrier native.

### 3. Build Carrier SDK

After importing dependencies from Carrier native, you need Android Studio to open this project and build Carrier Android SDK.

### 4. Output

After building with success, the output dist named **org.elastos.carrier-debug(release).aar** carrying jar package and JNI shared libraries to different CPU arch would be put under the directory:

```
app/build/outputs/aar
```

## Basic Tests

All basic tests are located under directory **"app/src/androidTest"**. You can run the tests on Android Studio. Before running tests, you need to uncomment **"service"** configuration in AndroidMinifest.xml.

## Build Docs

Open **Tools** tab on Android Studio and click **Generate JavaDoc...** item to generate the Java API document.

## Thanks

Sincerely thanks to all teams and projects that we relies on directly or indirectly.

## Contributing

We welcome contributions to the Elastos Carrier Android Project (or Native Project) in many forms.

## License

Elastos Carrier Android Project source code files are made available under the MIT License, located in the LICENSE file.
