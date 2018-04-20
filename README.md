# Elastos is Blockchain Powered Internet


Welcome to Elastos! 

What is Elastos? [Short Video 1:38](https://vimeo.com/264996792/d5265a3446)

[Elastos Introduction slides](https://www.slideshare.net/RongChen34/elastos-intro)

## Table of Contents

- [Overview](#overview)
  - [Quick Summary](#quick-summary)
- [How Elastos Works](#how-elastos-works)
  - [Elastos White Papers](#elastos-papers)
  - [Current State of Elastos](#current-states-of-elastos)
- [Community](#project-and-community)
- [Projects](#project-links)
  - [List of Repos](#list-of-repos)
- [License](#license)

## Overview

### Elastos
The entry point of whole Elastos github repos. 
It contains index of documents, tutotials, project links etc.

### Elastos.RT
Elastos Runtime as an App Engine or VM. It runs on top of existing OS, such as Linux, Android, iOS. It provides SDK for DApps to be running on top of RT.

As for the DApp developers, they do not need to care too much about the under-layer (blockchain) technical detail. They just need to call the RT.SDK. Building DApp will be much eaiser than before.
A [video demo for Elasltos RT](https://youtu.be/skewtq-kUQY) running on iOS (Mar 19, 2018). [Source code]( https://github.com/elastos/Elastos.RT/tree/master/Sources/Elastos/LibCore/tests/HelloCarDemo)


### Elastos.NET
Elastos.NET contains all of communication components of Elastos.RT. Including 
- Elastos.NET.Carrier: A P2P communication layer. 
- Elastos.NET.Carrier.Native.SDK: Native SDK for Carrier. 
- Elastos.NET.Carrier.Andorid.SDK: Android SDK for Carrier. A wrapper of Native SDK for Android developers.
- Elastos.NET.Carrier.IOS.SDK: IOS SDK for Carrier. A wrapper of Native SDK for IOS developers.
- Elastos.NET.Carrier.Nodejs.SDK: Nodejs SDK for Carrier. A Node.js Addon for Node.js developers.
- Elastos.NET.Carrier.Bootstrap: This is the basic service to help new node join and bootstrap the Elastos Carrier network. Consider it is the seed nodes.

### Elastos.Trinity
A Elastos web browser project still in stealth mode and will be released public shortly. It is the home for Web DApps.

### Elastos.ELA
Elastos blockchain projects. Including
- Elastos.ELA: This project is the source code that can build a full node of ELA.
- Elastos.ELA.Pay: ELA payment tool. Some documents are partially in Chinese. English version will be updated shortly.
- Elastos.ELA.Utilities.Java: This is a tool for generating transaction signature. Supports both API and HTTP.
- Other ELA projects will be open to public soon.

### Elastos.OS
Elastos is a development framework which can be running on hardware directly.
The programming idea with CAR is the essence technology in Elastos, it runs through the entire technology system implementation. In Elastos both the operating system kernel and the component library provided by Elastos platform are implemented using CAR technology. Closely integrated of the operating system kernel with CAR technology runtime environment can provide strong support to Elastos architecture.


### Quick Summary

## How Elastos Works

### Elastos White Papers

### Current State of Elastos

**Elastos is a work in progress!**

## Community
### What kind of community members are you hiring? [We Want You! Elastos Community Recruitment](https://medium.com/elastos/we-want-you-elastos-community-recruitment-da0e97694f63)
### How to contribute to the community? [Contribution Guide](./CONTRIBUTING.md)
### How to raise an issue? [Issue Template](./ISSUE_TEMPKLATE.md)
### Where are Elastos Developer Community Home?[Elastos Developer Community Repo](https://github.com/elastos/Elastos.Community)
### Where can I find Elastos Developer Community in my own city? [Elastos Developer Community Global](https://github.com/elastos/Elastos.Community.Global)

## Projects

### List of Repos

## License

Apache License Version 2.0 see [http://www.apache.org/licenses/LICENSE-2.0.html](http://www.apache.org/licenses/LICENSE-2.0.html)
