Elastos Carrier iOS Framework
=============================

## Summary

Elastos Carrier iOS Framework is swift API wrapper (and Objective-C APIs) for Elastos Native Carrier, where Carrier is a decentralized peer to peer communication framework.

## Build from source

### 1.Build Carrier NDK

You need to build carrier iOS ndk distributions from the Carrier native repository with following github address.

```
https://github.com/elastos/Elastos.NET.Carrier.Native.SDK
```

Finished building iOS NDKs for Carrier, you would have native output libraries **'lipo'ed** with serveral CPU architectures supported. Currently, only **x86-64** and **arm64** CPU architectures are supported.

The output static libraries would be listed under **"_dist/lipo"** directory in Carrier Native source.

### 2.Import Carrier NDK

The directory **"NativeDistributions"** to import native libraries and headers should have following directory structure:

```
NativeDistributions
   |--include
       |--ela_carrier.h
       |--ela_session.h
       |--CCarrier.swift
       |--CSession.swift
   |--libs
       |--libelacarrier.a
       |--libelacommon.a	
       |--libelasession.a	
       |--libflatcc.a	
       |--libflatccrt.a	
       |--libpj.a		
       |--libpjlib-util.a	
       |--libpjmedia.a	
       |--libpjnath.a	
       |--libsodium.a	
       |--libtoxcore.a
```
The headers under subdirectory **"include"** are public header files from Carrier native.

### 3. Build Carrier SDK

After importing dependencies from Carrier native, you need Apple Xcode to open this project and build Carrier iOS SDK.

### 4. Output

Use Apple Xcode to generate **ElastosCarrier.framework**.

## Tests

To complete.

## Build Docs

### 1. Swift APIs Docs

Run following script command to generate swift APIs docuement with appledoc tool:

```shell
$ ./docs.sh

```

About How to install appledoc, please refer to following github repository:

```
https://github.com/tomaz/appledoc

```

### 2. Object-C APIs Docs

To complete

## Thanks

Sincerely thanks to all teams and projects that we relies on directly or indirectly.

## Contributing

We welcome contributions to the Elastos Carrier iOS Project (or Native Project) in many forms.

## License

Elastos Carrier iOS Project source code files are made available under the MIT License, located in the LICENSE file.
