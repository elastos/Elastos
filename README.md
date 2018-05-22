# Elastos is Blockchain Powered Internet


Welcome to Elastos! 

What is Elastos? [Short Video 1:38](https://vimeo.com/264996792/d5265a3446)

Scripts:
>Current blockchains are designed to record transactions, not to store data. There is simply not enough space on the current blockchain to store a large quantity of digital movies and books. So they get congested.

>Elastos’ solution is a flexible mainchain and sidechain. The main chain is only responsible for basic transactions and transfer payments, while the sidechain executes smart contracts to support various applications and services. And
prevent overload.

>Elastos operating system runs as a flexible layer around the already congested blockchains. This method is also more secure. Identification and authentication come from the blockchain ID and is transferred to Elastos Runtime.

>Elastos will be a platform for decentralized applications (Dapps) that runs on a peer-to-peer network with no centralized control. An environment where digital assets (like copyrights managed games, music, movies and books,) can be traded peer to peer, safely. The creators and the users are protected on Elastos. And we don’t take (countdown percentage… 50-0%) ANY cut form these

>Welcome to the Elastos smartweb! Own your own data… and monetise it if you want.


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
The entry point for all the Elastos github repos. 
It contains index of documents, tutorials, project links etc.

### Elastos.RT
Elastos Runtime as an App Engine or VM. It runs on top of existing OS, such as Linux, Android, iOS. It provides SDK for DApps to be running on top of RT.

As for the DApp developers, they do not need to worry too much about the technical details of the layer underneath(blockchain layer). They just need to call the RT.SDK. Building DApps will be much eaiser than before.
- A [video demo for Elasltos RT](https://youtu.be/skewtq-kUQY) running on iOS (Mar 19, 2018). [Source code]( https://github.com/elastos/Elastos.RT/tree/master/Sources/Elastos/LibCore/tests/HelloCarDemo)


### Elastos.NET
Elastos.NET contains all of communication components of Elastos.RT that includes:
- Elastos.NET.Carrier: A P2P communication layer
- Elastos.NET.Carrier.Native.SDK: Native SDK for Carrier
- Elastos.NET.Carrier.Andorid.SDK: Android SDK for Carrier. A wrapper of Native SDK for Android developers
- Elastos.NET.Carrier.IOS.SDK: IOS SDK for Carrier. A wrapper of Native SDK for IOS developers
- Elastos.NET.Carrier.Nodejs.SDK: Nodejs SDK for Carrier. A Node.js addon for Node.js developers
- Elastos.NET.Carrier.Bootstrap: This is the basic service to help new nodes join and bootstrap the Elastos Carrier network. You can think of these nodes as the seed nodes

### Elastos.Trinity
An Elastos web browser project still in stealth mode and will be released to the public shortly. It is home for Web DApps.

### Elastos.ELA
Elastos blockchain projects that includes:
- Elastos.ELA: This project is the source code that can build a full node of ELA.
- Elastos.ELA.Pay: ELA payment tool. Some documents are partially in Chinese. English version will be updated shortly.
- Elastos.ELA.Utilities.Java: This is a tool for generating transaction signature. Supports both API and HTTP.
- Other ELA projects will be open to public soon.

### Elastos.OS
Elastos is a development framework that can directly run on the hardware.
The programming idea with CAR is the essence technology in Elastos OS. It runs through the entire technology system implementation. In Elastos OS, both the operating system kernel and the component library provided by the Elastos platform are implemented using CAR technology. The operating system kernel is closely integrated with CAR technology so the runtime environment can provide strong support to Elastos architecture.


### Quick Summary

## How Elastos Works

### Elastos White Papers
- [Main WhitePapers in Multiple Languages](./Whitepapers/MainWhitepaper)
- [Sidechain WhitePapers in Multiple Languages](./Whitepapers/SidechainWhitepaper)

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
- [Elastos.OS](https://github.com/elastos/Elastos.OS)
- [Elastos.RT](https://github.com/elastos/Elastos.RT)
- [Elastos.ELA](https://github.com/elastos/Elastos.ELA)
  - [Elastos.ELA.SPV](https://github.com/elastos/Elastos.ELA.SPV)
    - [Elastos.ELA.SPV.Node](https://github.com/elastos/Elastos.ELA.SPV.Node)
  - [Elastos.ELA.Client](https://github.com/elastos/Elastos.ELA.Client)
  - [Elastos.ELA.Utilities.Java](https://github.com/elastos/Elastos.ELA.Utilities.Java)
- [Elastos.ELA.Utility](https://github.com/elastos/Elastos.ELA.Utility)
- [Elastos.NET.Carrier.Native.SDK](https://github.com/elastos/Elastos.NET.Carrier.Native.SDK)
  - [Elastos.NET.Carrier.Nodejs.SDK](https://github.com/elastos/Elastos.NET.Carrier.Nodejs.SDK)
  - [Elastos.NET.Carrier.iOS.SDK](https://github.com/elastos/Elastos.NET.Carrier.iOS.SDK)
  - [Elastos.NET.Carrier.Android.SDK](https://github.com/elastos/Elastos.NET.Carrier.Android.SDK)
    - [Elastos.NET.Carrier.Samples.Android](https://github.com/elastos/Elastos.NET.Carrier.Samples.Android)
  - [Elastos.NET.Carrier.Bootstrap](https://github.com/elastos/Elastos.NET.Carier.Bootstrap)
- [Elastos.Trinity](https://github.com/elastos/Elastos.Trinity)
  - [Elastos.Trinity.Android](https://github.com/elastos/Elastos.Trinity.Android)
- [Elastos.DittoBox.Server](https://github.com/elastos/Elastos.DittoBox.Server)
  - [Elastos.DittoBox.iOS](https://github.com/elastos/Elastos.DittoBox.iOS)
  - [Elastos.DittoBox.Android](https://github.com/elastos/Elastos.DittoBox.Android)
- [Elastos.Community](https://github.com/elastos/Elastos.Community)
  - [Elastos.Community.Global](https://github.com/elastos/Elastos.Community.Global)

## License

Apache License Version 2.0 see [http://www.apache.org/licenses/LICENSE-2.0.html](http://www.apache.org/licenses/LICENSE-2.0.html)
