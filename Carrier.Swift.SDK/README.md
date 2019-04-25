Elastos Carrier Swift Framework
=============================

[![Build Status](https://travis-ci.org/elastos/Elastos.NET.Carrier.Swift.SDK.svg?)](https://travis-ci.org/elastos/Elastos.NET.Carrier.Swift.SDK)

## Summary

Elastos Carrier Swift Framework is swift API wrapper (and Objective-C APIs) for Elastos Native Carrier, where Carrier is a decentralised peer to peer communication framework.

## Build from source

### 1.Build Carrier NDK

You need to build carrier iOS/Macos ndk distributions from the Carrier native repository with following github address.

```
https://github.com/elastos/Elastos.NET.Carrier.Native.SDK
```

Finished building iOS/Macos NDKs for Carrier, you would need native output libraries **'lipo'ed** with serveral CPU architectures supported. Currently, only **x86-64** and **arm64** CPU architectures are supported.

The output static libraries would be listed under **"_dist/lipo"** directory in Carrier Native source.

### 2.Import Carrier NDK

The directory **"NativeDistributions"** to import native shared libraries and headers should have following directory structure:

```
NativeDistributions
   |--include
       |--ela_carrier.h
       |--ela_session.h
       |--CCarrier.swift
       |--CSession.swift
   |--libs
       |--libcrystal.dylib
       |--libelacarrier.dylib
       |--libelasession.dylib
```

The headers under subdirectory **"include"** are public header files from Carrier native.

### 3. Build Carrier SDK

After importing dependencies from Carrier native, you need Apple Xcode to open this project and build Carrier iOS/Macos SDK.

### 4. Output

Use Apple Xcode to generate **ElastosCarrierSDK.framework**.

## Tests

TODO.

## CocoaPods

### 1. Pod install
The distribution has been published to CocoaPods platform. So, the simple way to use **ElastosCarrierSDK.framework** is just to add the following line to your **Podfile**:

```
  pod 'ElastosCarrierSDK'
```

Then run the command below to install it before open your Xcode workspace:
```shell
$ pod install
```

### 2. Add shell script for codesign

In the project navigator, select the tab **Build Phases**  in the specific  **TARGET" ** item. Then click **+** button on the right-top corner to choose **New Run Script Phase** and expand it to add shell script as below:

```
 "${SRCROOT}/Pods/ElastosCarrierSDK/ElastosCarrier-framework/CocoaPods/codesigncarrierframework.sh"
```

## Build Docs

### 1. Swift APIs Docs

Run following script command to generate swift APIs document with appledoc tool:

```shell
$ ./docs.sh

```

About How to install appledoc, please refer to following github repository:

```
https://github.com/tomaz/appledoc

```

### 2. Object-C APIs Docs

TODO.

## Thanks

Sincerely thanks to all teams and projects that we relies on directly or indirectly.

## Contributing

We welcome contributions to the Elastos Carrier Swfit Project (or Native Project) in many forms.

## License

Elastos Carrier Swift Project source code files are made available under the GPLv3 License, located in the LICENSE file.
