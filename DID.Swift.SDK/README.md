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

You need to build DID iOS ndk distributions from the DID native repository with following github address.

```
https://github.com/elastos/Elastos.NET.DID.Native.SDK
```

Finished building iOS NDKs for DID, you would need native output libraries **'lipo'ed** with serveral CPU architectures supported. Currently, only **x86-64** and **arm64** CPU architectures are supported.

The output static libraries would be listed under **"_dist/lipo"** directory in DID Native source.

### 2.Import DID NDK

The directory **"Externals/HDKey"** to import native shared libraries and headers should have following directory structure:

```
Externals/HDKey
   |--include
       |--crypto.h
       |--HDkey.h
       |--CHDKey.swift
   |--libs
       |-- PLACEHOLDER.md
       （Please read the PLACEHOLDER.md）
```

The headers under subdirectory **"include"** are public header files from DID native.

### 3. Build DID SDK

After importing dependencies from DID native, you need Apple Xcode to open this project and build DID iOS SDK.

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



