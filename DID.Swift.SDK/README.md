Elasots.DID.Swift.SDK
===================

## Introduction

**Elastos DID (Decentralized Identifier) Swift framework** is a set of Swift APIs for Elastos DID used by dApps on iOS/MacOS platforms, where Elastos DID is the DID system of Elastos ecosystem with compatibility to W3C DIDs specs. 

> Decentralized Identifiers (DIDs) are a new type of identifier for verifiable, decentralized digital identity. These new identifiers are designed to enable the controller of a DID to prove control over it and to be implemented independently of any centralized registry, identity provider, or certificate authority. DIDs are URLs that relate a DID subject to means for trustable interactions with that subject. DIDs resolve to DID Documents — simple documents that describe how to use that specific DID. Each DID Document may express cryptographic material, verification methods, and/or service endpoints. These provide a set of mechanisms which enable a DID controller to prove control of the DID. Service endpoints enable trusted interactions with the DID subject.
> 
>> <cite> https://w3c-ccg.github.io/did-spec/</cite> 

## Build from source

Use the following commands to download and build source code:

```shell
$ git clone https://github.com/elastos/Elastos.DID.Swift.SDK.git
$ cd Elastos.DID.Swift.SDK
$ pod install
$ open -a Xcode ElastosDIDSDK.xcworkspace
```


### 1.Build DID NDK

Before to build the whole workspace, native library distributions targeting at iPhone device should be built from DID native repository with following commands on macOS:

```
$ git clone https://github.com/elastos/Elastos.DID.Native.SDK.git
$ cd Elastos.DID.Native.SDK/build
$ mkdir -p iphoneos
$ cd iphoneos
$ cmake -DHDKEY_ONLY=ON \
        -DIOS_PLATFORM="iphoneos" \
        -DCMAKE_TOOLCHAIN_FILE=../../cmake/iOSToolchain.cmake \
        ../..
$ make install
```

Where you also can use **iphonesimulator** option to build for iPhone simulator under new directory with appropriate name:

```
$ cd Elastos.DID.Native.SDK/build
$ mkdir -p iphonesimulator
$ cd iphoneosimulator
$ cmake -DHDKEY_ONLY=ON \
        -DIOS_PLATFORM="iphonesimulator" \
        -DCMAKE_TOOLCHAIN_FILE=../../cmake/iOSToolchain.cmake \
        ../..
$ make install
```

Noticed that only the architectures of **x86-64** (simulator) and **am64** (device) are supported.

### 2.Import DID NDK

The directory **"Externals/HDKey"** would rely on native shared libraries built from upper step, and should have following directory structure:

```
Externals/HDKey
   |--include
       |-- crypto.h
       |-- HDkey.h
       |-- CHDKey.swift
   |--libs
       |--PLACEHOLDER.md
       （Please read the PLACEHOLDER.md）
```

Then follow the instruction of **PLACEHOLDER.md** , and import native headers and libraries with the specific locations.

### 3. Build DID SDK

Once all native dependencies are ready,  use **Apple Xcode** to open the workspace and start to build for **Swift DID SDK**.

### 4. Output

Use Apple Xcode to generate **ElastosDIDSDK.framework**.

## Tests

### 1.Import DID NDK

The directory **"Externals/SPVWrapper/SPVWrapper"** to import native shared libraries and headers should have following directory structure:

```
Externals/SPVWrapper/SPVWrapper
   |--include
       |--spvadapter.h
       |--CSPVWrapper.swift
   |--libs
       |-- PLACEHOLDER.md
       （Please read the PLACEHOLDER.md）
```

### 2. run unit tests
After importing dependencies from DID native, you can run unit tests.

## Usage 

COMING SOON

## Build Docs

COMING SOON

## Thanks

Sincerely thanks to all teams and projects that we relies on directly or indirectly.

## Contributing

Welcome contributions to the Elastos DID SDK in many forms.

## License

MIT 



