Elastos Carrier Android SDK
===========================
## Summary

Elastos Carrier Android SDK is java api wrapper for Elastos Native Carrier, where Carrier is a decentralized peer to peer communication framework.

## Build from source

### 1.Build Carrier NDK

You need to build carrier android ndk distributions from the Carrier native repository with following github address.

```
https://github.com/elastos/Elastos.NET.Carrier.Native.SDK
```

Finished building android ndk for Carrier, you would have native output library **libcarrier-native.a** to each CPU arch, currently supported for **armv7l**, **arm64**, **x86**, **x86-64**.

### 2.Import Carrier NDK

The directory **"app/native-dist"** to import native libraries and headers should have following directory structure:

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

The headers under subdirectory **"include"** are public header files exported from Carrier native. And **"libcarrier-native.a"** is just a static library dist to each CPU arch from Carrier native.

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