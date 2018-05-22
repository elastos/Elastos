# Elastos is Blockchain Powered Internet


Welcome to Elastos! 

What is Elastos? [Short Video 1:38](https://vimeo.com/264996792/d5265a3446)

Script:
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
  - [Elastos White Papers](#elastos-white-papers)
  - [Current State of Elastos](#current-state-of-elastos)
- [Community](#community)
- [Projects](#projects)
  - [List of Repos](#list-of-repos)
- [Resources](#resources)
- [License](#license)

## Overview

### Quick Summary

With 18 years of development, Elastos is the first completely secure and decentralized environment on the internet, offering a revolutionary C++ operating system in two forms. The first is a general-purpose OS that because of its agnostic environment, can be directly installed on any hardware in the world, giving unparalleled security to self-driving cars, AR/VR platforms, internet of things devices, smart phones, and smart routers. The second is the Elastos runtime, a sandbox environment that can be installed on your smart phone as an application from the app store. Built with blockchain, Elastos creates a virtual ecosystem where decentralized applications are protected from direct access with the internet while allowing near infinite scalability to billions of users. 
Elastos allows content creators to create an economy around their work and allows developers to actually ‘write once, run anywhere’—applications are equipped with maximum security, built with high scalibility and run with total decentralization.

## How Elastos Works

### Elastos White Papers
- [Main WhitePapers in Multiple Languages](./Whitepapers/MainWhitepaper)
- [Sidechain WhitePapers in Multiple Languages](./Whitepapers/SidechainWhitepaper)

### Current State of Elastos

**Elastos is a work in progress!**

## Projects

### List of Repos

#### [Elastos.OS](https://github.com/elastos/Elastos.OS)
Elastos is a development framework that can directly run on the hardware.
The programming idea with CAR is the essence technology in Elastos OS. It runs through the entire technology system implementation. In Elastos OS, both the operating system kernel and the component library provided by the Elastos platform are implemented using CAR technology. The operating system kernel is closely integrated with CAR technology so the runtime environment can provide strong support to Elastos architecture.

#### [Elastos.RT](https://github.com/elastos/Elastos.RT)
Elastos Runtime as an App Engine or VM. It runs on top of existing OS, such as Linux, Android, iOS. It provides SDK for DApps to be running on top of RT. As for the DApp developers, they do not need to worry too much about the technical details of the layer underneath(blockchain layer). They just need to call the RT.SDK. Building DApps will be much eaiser than before.

#### [Elastos.ELA](https://github.com/elastos/Elastos.ELA)
ELA is the digital currency solution within Elastos ecosystem. This project is the source code that can build a full node of ELA.

#### [Elastos.ELA.SPV](https://github.com/elastos/Elastos.ELA.SPV)
Elastos SPV is a SDK of SPV (Simplified Payment Verification) implementation of the Elastos digital currency. The Elastos SPV SDK is a set of encryption algorithm, peer to peer network and SPV related implementation like bloom filter, merkleblock and util methods. As an example, this project include a spv wallet implementation located in spvwallet folder, it will help you understand how to use this SDK and build your own apps. The flowing instructions will help you get into the SDK and build up the spvwallet sample APP and play with it.

#### [Elastos.ELA.SPV.Node](https://github.com/elastos/Elastos.ELA.SPV.Node)
This project is to implement an ELA node like program base on ELA.SPV SDK, it provides the same RPC interfaces as the ELA full node program like getblock gettransaction etc, and several extra interfaces registeraddresses registeraddress. With a SPV node, you can do almost the same thing as an ELA full node through JSON-RPC interaction, with reduced data size and less computing resource.

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
- [Demos](./Demos)
- [Media](./Media)

## License

Apache License Version 2.0 see [http://www.apache.org/licenses/LICENSE-2.0.html](http://www.apache.org/licenses/LICENSE-2.0.html)
