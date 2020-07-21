Elastos DID Java SDK
==================

[![Build Status](https://travis-ci.com/elastos/Elastos.DID.Java.SDK.svg?)](https://travis-ci.com/elastos/Elastos.DID.Java.SDK)

## Introduction

This repository contains the Java SDK implementation of W3C DID and Verifiable Claims specifications with DLT of Elastos DID SideChain. And this implementation of Java SDK also works on android platform.

The implementation of Elastos DID conforms to W3C's DID specification and would be used by the Elastos ecosystem to provide decentralised identification capabilities to Elastos developers and end users.

## Build from source

Get the source code for this repository and build it with commands listed below:

```
$ git clone https://github.com/elastos/Elastos.DID.Java.SDK
$ cd Elastos.DID.Java.SDK
$ ./gradlew build -x test
```

Another choice to use **Eclipse** or **Android Studio** to import this grade project to make build and run tests.

## Publish the Maven package

Get the source code for this repository and create Maven package with commands listed below:

```
$ git clone https://github.com/elastos/Elastos.DID.Java.SDK
$ cd Elastos.DID.Java.SDK
$ ./gradlew -Prelease -Pversion=1.2.3 publish
```

You can change the `1.2.3` to the real version code. The Maven publications can be located at build/repos/*version*/ directory.

## Tests

TODO

## Usage

TODO

## Thanks

Sincerely thanks to all teams and projects that we relies on directly or indirectly.

## Contributing

Welcome contributions to the Elastos DID SDK in many forms.

## License

MIT@elastos.org

