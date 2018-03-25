# Elastos Carrier Android SDK

## Summary

Elastos Carrier Android SDK is java api wrapper for Elastos Native Carrier, where Carrier is a decentralized peer to peer communication framework.

## Build from source

### 1.Build Carrier NDK

You need to build carrier android ndk distributions from the Carrier native repository with following github address.

```
https://github.com/elastos/Elastos.NET.Carrier.Native.SDK
```

Finished building android ndk for Carrier, you would have native output libraries (libcarrier-native.a) to each CPU arch, such as armv7l, arm64, x86, x86-64 and etc.

### 2.Import Carrier NDK

The directory "app/native-dist" where to import native libraries and headers has the following directory structure:

```
app/native-dist
   |--include
       |--ela_carrier.h
       |--ela_session.h
   |--libs
       |--armeabi
          |--libcarrier-native.a
       |--armeabi-v7a
          |--libcarrier-native.a
       |--arm64-v8a
          |--libcarrier-native.a
       |--x86
          |--libcarrier-native.a
       |--x86-64
          |--libcarrier-native.a
```

The headers under directory "include" are public header files from Carrier native. And "libcarrier-native.a" is just a static library dist to each CPU arch from Carrier native.

### 3. Build Carrier SDK

After importing dependencies from Carrier native, you need Android Studio to open this project and build Carrier Android SDK.

### 4. Output

The output distributions after build include Carrier jar package and JNI shared libraries to each CPU arch, where the structure listed below:

```
build/outputs
	|--elacarrier.jar
	|--libs
		|--armeabi
          |--libcarrierjni.so
       |--armeabi-v7a
          |--libcarrierjni.so
       |--arm64-v8a
          |--libcarrierjni.so
       |--x86
          |--libcarrierjni.so
       |--x86-64
          |--libcarrierjni.so
```

## Basic Tests

All basic tests are located under directory "app/src/androidTest". And you can run the tests on Android Studio.

## Build Docs

Open "Tools" tab on Android Studio and click "Generate JavaDoc..." item to produce the Java API document.

## Thanks

Sinserely thanks to all teams and projects that we relying on directly or indirectly.

## Contributing

We welcome contributions to the Elastos Android Project (or Native Project) in many forms.

## License

Elastos Carrier Android Project source code files are made available under the MIT License, located in the LICENSE file. 