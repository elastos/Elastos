This folder is hold for native dependency libraries with layout below:

```
|-- -iphoneos
    |-- libboost_filesystem.a
    |-- libboost_system.a
    |-- libcjson.a
    |-- libcrypto.a
    |-- libcrystal.a
    |-- libcurl.a
    |-- libfruit.a
    |-- libspdlog.a
    |-- libspvadapter.a
    |-- libsqlite3.a
    |-- libssl.a
    |-- libtls.a
    |-- libz.tbd
    |-- libc++.1.tbd
    |-- libc++.tbd
    |-- libc++abi.1.tbd
|-- -iphonesimulator
    |-- libboost_filesystem.a
    |-- libboost_system.a
    |-- libcjson.a
    |-- libcrypto.a
    |-- libcrystal.a
    |-- libcurl.a
    |-- libfruit.a
    |-- libspdlog.a
    |-- libspvadapter.a
    |-- libsqlite3.a
    |-- libssl.a
    |-- libtls.a
    |-- libz.tbd
    |-- libc++.1.tbd
    |-- libc++.tbd
    |-- libc++abi.1.tbd
```

**Notice**:

And if you want build framework used for macos platform, then added relevant native libraries:

```
|-- -iphoneos
|-- -iphonesimulator
|-- macosx
    |-- libboost_filesystem.a
    |-- libboost_system.a
    |-- libcjson.a
    |-- libcrypto.a
    |-- libcrystal.a
    |-- libcurl.a
    |-- libfruit.a
    |-- libspdlog.a
    |-- libspvadapter.a
    |-- libsqlite3.a
    |-- libssl.a
    |-- libtls.a
    |-- libz.tbd
    |-- libc++.1.tbd
    |-- libc++.tbd
    |-- libc++abi.1.tbd
```
