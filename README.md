# Elastos is a Blockchain Powered Internet


Welcome to Elastos! 

What is Elastos? [Short Video 1:49](https://www.youtube.com/watch?v=20mTY3G5y0c)

Script:
>Blockchains are ideal for recording transactions but not for storing data. There is simply not enough space to store large numbers of files which means the blockchain easily gets congested. To prevent overload, Elastos provides the flexible main chain-sidechains design structure. The main chain is responsible for necessary transactions and transfer payments while the sidechains execute smart contracts to support various applications and services. The Elastos Operating System runs as a highly secure flexible layer around the blockchain to free up more space. Identification and authentication come from the blockchain ID which are then transferred to the elastos runtime OS. The Elastos OS has been in development for 18 years and has over 10 million lines of code. To put this into perspective, the space shuttle needed 400,000, android took 11.8 million and the F35 required 24.7 million lines of code. The Elastos Platform targets decentralized applications that run on a peer to peer network with no centralized control. It provides an environment where digital assets can be traded safely. Both creators and users are protected by Elastos. Welcome to the Elastos Smart Web - own your data. 

[Elastos Introduction slides](https://www.slideshare.net/RongChen34/elastos-intro)

## Table of Contents

- [Overview](#overview)
  - [Quick Summary](#quick-summary)
- [How Elastos Works](#how-elastos-works)
  - [Elastos White Papers](#elastos-white-papers)
  - [Beginner Guides](#beginner-guides)
  - [Current State of Elastos](#current-state-of-elastos)
- [Community](#community)
- [Projects](#projects)
  - [List of Repos](#list-of-repos)
- [Resources](#resources)
- [License](#license)

## Overview

### Quick Summary

Elastos provides a virtual ecosystem where applications run in a completely safe sandboxed environment whose authenticity is verified using the Blockchain and where the flow of traffic is completely decentralized using a peer to peer network.

Elastos solves the problem of security using the trusted runtime environment, scalability using the main chain sidechains solutions and decentralization using the peer to peer network.

Elastos is not a typical operating system. Usually, when you think about operating systems, you think abut a device operating system that is tied to a particular device. Elastos is not a device OS but rather it’s a network operating system. This means the network is the computer instead of device being the computer. This also means that this operating system will never need to be upgraded because the network has no version. It’s just the network. The internet has no version. It’s just the internet. The internet gets upgraded by adding services on top of it and people using those services. Similarly, the elastos network OS gets upgraded when someone adds a new service and people start using this service. There is no human intervention required as everything is code and everything is autonomous and self running. The elastos network operating system is literally a world supercomputer formed of millions and millions of virtual machines spread across the internet where your data might be stored in ur own personal disk or IPFS(which is distributed across the internet) where there are no more device drivers needed because all the code gets generated for the appropriate device on the fly automatically. In that sense, every device on the internet will be able to recognize elastos because the internet itself is the device.

## How Elastos Works

### Elastos White Papers
- [Main WhitePapers in Multiple Languages](./Whitepapers/MainWhitepaper)
- [Sidechain WhitePapers in Multiple Languages](./Whitepapers/SidechainWhitepaper)

### Beginner Guides

**A Beginner's Guide to Elastos for Non-Developers**

This is material for any non-developer who wants to learn about Elastos and what it has to offer to an end-user/consumer/investor. This guide explains the Elastos core technology and how it differs from various other projects in the blockchain industry. First and foremost, it's important to note that Elastos is not a blockchain project. It is a network operating system project powered by the blockchain technology so in that sense, Elastos is not competing with any other blockchain projects but can work together with them to form this new ecosystem where the DApps run directly on the device instead of running on the blockchain along with decentralized peer to peer network to transfer assets in a completely closed sandboxed environment, thereby solving the trilemma of the blockchain technology - security, scalibility and decentralization all in one. 

[A Non-Developer guide](https://github.com/elastos/Elastos/wiki/A-Non-Developer-Guide-to-Elastos)

**A Beginner's Guide to Elastos for Developers**

This is material for any developer who has some experience with programming languages like Javascript/NodeJS, HTML5, Java, Swift, C++, Golang, etc and wants to get started with various github projects that are open sourced by Elastos. This includes the core development of the product such as the Elastos Runtime environment, Elastos Blockchain, creating and integrating ELA into wallets, or creating applications using the SDK if you're a DApp developer. In order to make it very easy for any developer(even to someone who's not very familiar with blockchain), Elastos provides the SDK in various languages that makes it easy to start developing a decentralized application integrated with blockchain technology without having to fully learn about how blockchain works underneath.

[A Developer guide](https://github.com/elastos/Elastos/wiki/A-Developer-Guide-to-Elastos)

### Current State of Elastos

**Elastos is a work in progress!**

## Projects

### List of Repos

#### [Elastos.OS](https://github.com/elastos/Elastos.OS)
Elastos is a general-purpose operating system for intelligent terminals. It is based on Android's open resources and is developed using CAR component technology. The Elastos operating system has a system architecture similar to WinRT (Windows Runtime) and supports C++, Java, and JavaScript application development with a consistent application model. 

Elastos is a C++ operating system that can run directly on the hardware - eg. IoT devices, smart phones, routers, AR/VR headsets, etc. The operating system kernel is implemented using CAR technology. The programming idea with CAR is the essence technology in Elastos OS. It runs through the entire technology system implementation. Components from different sources can interoperate(java can call javascript code, javascript can call C++, etc). 

#### [Elastos.RT](https://github.com/elastos/Elastos.RT)
Elastos Runtime can be thought of as an App Engine or a Virtual Machine(VM). It is a runtime sandboxed closed environment that runs on top of existing OS, such as Android, iOS, Linux, etc. It provides SDK for DApps to be running on top of RT. As for the DApp developers, they do not need to worry too much about the technical details of the layer underneath(blockchain layer). They just need to call the RT.SDK. Building DApps will be much eaiser than before. DApp developers will be using Cordova to develop their HTML5 applications that will be running on trinity browser. Non-Elastos apps can access the Elastos Smart Web via the RT SDK(C++ SDK) because android and iOS apps can call C++ SDK. This is done to make it easier for existing mobile developers to integrate their existing mobile apps with Elastos.

#### [Elastos.ELA](https://github.com/elastos/Elastos.ELA)
ELA is the digital currency solution within Elastos ecosystem. This project is the source code that can build a full node of ELA.

#### [Elastos.ELA.SPV](https://github.com/elastos/Elastos.ELA.SPV)
Elastos SPV is a SDK of SPV (Simplified Payment Verification) implementation of the Elastos digital currency. The Elastos SPV SDK is a set of encryption algorithm, peer to peer network and SPV related implementation like bloom filter, merkleblock and util methods. As an example, this project includes an spv wallet implementation located in spvwallet folder. It will help you understand how to use this SDK and build your own apps. After installing, you can do some things locally like creating your own wallet, seeing account balance and a wide variety of other options.

#### [Elastos.ELA.SPV.Node](https://github.com/elastos/Elastos.ELA.SPV.Node)
This project implements an ELA node like program base on ELA.SPV SDK. It provides the same RPC interfaces as the ELA full node program like getblock gettransaction etc, and several extra interfaces registeraddresses, registeraddress, etc. With a SPV node, you can do almost the same thing as an ELA full node through JSON-RPC interaction, with reduced data size and less computing resource.

#### [Elastos.ELA.Client](https://github.com/elastos/Elastos.ELA.Client)
This is the client program of the ELA node, which is a command line tool to control node and see node info etc. Also, this project includes a light implementation of ELA wallet that can create your ELA account, receive, create, sign or send transactions. You can run a node locally and set the miner address to your wallet account and then run the node to get your own ELAs and do whatever you want after that.

#### [Elastos.ELA.Utilities.Java](https://github.com/elastos/Elastos.ELA.Utilities.Java)
This is a set of tools and utilities for generating a transaction signature. THere are two forms of signature - calling the API and standard web request.

#### [Elastos.ELA.Utility](https://github.com/elastos/Elastos.ELA.Utility)
This is a common library referenced by all the repos of ELA nodes and clients, including Elastos.ELA, Elastos.ELA.Client, Elastos.ELA.SPV, etc. It is the basic component of these repos. This repo contains some common data structures like the message protocol of p2p network and the crypto algorithm of ELA coin. Changes to this repo will affect all the repos that reference it. Thus, any new commit to this repo should be treated very cautiously.

#### [Elastos.NET.Carrier.Native.SDK](https://github.com/elastos/Elastos.NET.Carrier.Native.SDK)
Elastos Carrier is a decentralized peer to peer communication framework and this repository is the Native SDK from which all other SDKs are based on.

#### [Elastos.NET.Carrier.Nodejs.SDK](https://github.com/elastos/Elastos.NET.Carrier.Nodejs.SDK)
This is the Node.js wrapper for Elastos Carrier Native. It allows javascript code to call Carrier C++ functions. This project is just a wrapper to the Native SDK for Node developers.

#### [Elastos.NET.Carrier.iOS.SDK](https://github.com/elastos/Elastos.NET.Carrier.iOS.SDK)
This is a Swift API wrapper(and Objective C APIs) for Elastos Carrier Native. This project is just an iOS wrapper to the Native SDK for iOS developers.

#### [Elastos.NET.Carrier.Android.SDK](https://github.com/elastos/Elastos.NET.Carrier.Android.SDK)
This is a Java API wrapper for Elastos Carrier Native. This project is just a wrapper to the Native SDK for Android developers.

#### [Elastos.NET.Carrier.Samples.Android](https://github.com/elastos/Elastos.NET.Carrier.Samples.Android)
This demo introduces how to use the Elastos Android SDK to achieve communication between phones and realizes a simple version of Chat DApp.

#### [Elastos.Carrier.Demo.Remoter.iOS](https://github.com/elastos/Elastos.Carrier.Demo.Remoter.iOS)
This is a demo application to show what can be done over carrier network. It shows you how you can use this app to control each other via what's known as NAT traversal. Some of the items for remote control includes - turning on/off torch(or light), turning on/off ringtone, increase/decrease ringtone volume, turning on/off camera, etc

#### [Elastos.NET.Carrier.Bootstrap](https://github.com/elastos/Elastos.NET.Carier.Bootstrap)
This is the basic service to help new nodes join and bootstrap the Elastos Carrier network. You can think of these nodes as the seed nodes.

#### [Elastos.Trinity](https://github.com/elastos/Elastos.Trinity)
Trinity is a project name that aims to implement a cross-platform application that can run on android, iOS, windows and mac OS. It is the entrance from the classical internet into the trusted elastos internet. The official application name is called "Elastos". This is an Elastos web browser project still in stealth mode and will be released to the public shortly. It is home for web DApps.

#### [Elastos.Trinity.Android](https://github.com/elastos/Elastos.Trinity.Android)
This projects hosts Trinity code for android.

#### [Elastos.DittoBox.Server](https://github.com/elastos/Elastos.DittoBox.Server)
DittoBox server integrates ownCloud Server and Elastos Carrier. You can access your files at anytime from anywhere over Elastos Carrier network even if the server is deployed behind the router.

#### [Elastos.DittoBox.iOS](https://github.com/elastos/Elastos.DittoBox.iOS)
This is an iOS demo application integrating ownCloud over Elastos Carrier network through which we can access or save personal files to ownCloud server that could be deployed at home behind the router. This app demonstrates that all traditional http(/https)-based application can be refactored to elastos carrier apps running over carrier network. Being elastos carrier web app, the app server can be deployed without requirement of direct network accessibility. For example, through elastos carrier network, you can deploy ownCloud server at local network at your home, and access ownCloud service at anywhere.

#### [Elastos.DittoBox.Android](https://github.com/elastos/Elastos.DittoBox.Android)
This is an android demo application integrating ownCloud over Elastos Carrier network through which we can access or save personal files to ownCloud server that could be deployed at home behind the router. This app demonstrates that all traditional http(/https)-based application can be refactored to elastos carrier apps running over carrier network. Being elastos carrier web app, the app server can be deployed without requirement of direct network accessibility. For example, through elastos carrier network, you can deploy ownCloud server at local network at your home, and access ownCloud service at anywhere.

## Community
- **What kind of community members are you hiring?** [We Want You! Elastos Community Recruitment](https://medium.com/elastos/we-want-you-elastos-community-recruitment-da0e97694f63)
- **How to contribute to the community?** [Contribution Guide](./CONTRIBUTING.md)
- **How to raise an issue?** [Issue Submission Checklist](./ISSUE_SUBMISSION_CHECKLIST.md)
- **Is there a gitlab repo dedicated to Elastos Developer Community?** [Elastos Developer Community Repo](https://github.com/elastos/Elastos.Community)
- **Can I find other Elastos developers in my city?** [Elastos Developer Community Global](https://github.com/elastos/Elastos.Community.Global)

## Resources
- [Articles and Blogs](./ArticlesAndBlogs)
- [Media](./Media)

## License

Apache License Version 2.0 see [http://www.apache.org/licenses/LICENSE-2.0.html](http://www.apache.org/licenses/LICENSE-2.0.html)
