This folder is hold for native dependency libraries with layout below:

```
|-- -iphoneos
    |-- libflatccrt.a
    |-- libtoxcore.a
    |-- libpj.a
    |-- libpjnath.a
    |-- libpjlib-util.a
    |-- libpjmedia.a
    |-- libcrystal.a
    |-- libelacarrier.a
    |-- libelasession.a
    |-- libelafiletrans.a
|-- -iphonesimulator
    |-- libflatccrt.a
    |-- libtoxcore.a
    |-- libpj.a
    |-- libpjnath.a
    |-- libpjlib-util.a
    |-- libpjmedia.a
    |-- libcrystal.a
    |-- libelacarrier.a
    |-- libelasession.a
    |-- libelafiletrans.a
```

**Notice**:

And if you want build framework used for macos platform, then added relevant native libraries:

```
|-- -iphoneos
|-- -iphonesimulator
|-- macosx
    |-- libflatccrt.a
    |-- libtoxcore.a
    |-- libpj.a
    |-- libpjnath.a
    |-- libpjlib-util.a
    |-- libpjmedia.a
    |-- libsrtp.a
    |-- libcrystal.a
    |-- libelacarrier.a
    |-- libelasession.a
    |-- libelafiletrans.a
```