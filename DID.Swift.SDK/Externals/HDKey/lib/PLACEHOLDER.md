This folder is hold for native dependency libraries with layout below:

```
|-- -iphoneos
    |-- libcrypto.a
    |-- libhdkey.a
|-- -iphonesimulator
    |-- libcrypto.a
    |-- libhdkey.a
```

**Notice**:

And if you want build framework used for macos platform, then added relevant native libraries:

```
|-- -iphoneos
|-- -iphonesimulator
|-- macosx
    |-- libcrypto.a
    |-- libhdkey.a
```
