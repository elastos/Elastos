---
title: Types of Elastos Applications
sidebar_label: Types of Apps
---

### Transitioning from Web Development to DApp Development

The first thing a new blockchain developer needs to understand is why the concept of a website is fundamentally
centralized. What separates a website and a Chrome extension like [Metamask](https://metamask.io)?


### A litmus test for centralized vs decentralized

**An easy way to think about centralization is whether there is a single off switch.** A website relies on a URL which 
points to a single DNS record and that in itself is a centralized feature, to take down a website a person only needs to
modify that DNS record.


##### Whereas the Ethereum blockchain has multiple nodes and no single off switch.

In this case how do we make a decentralized application? The first step is to have a name service that is decentralized.

In Ethereum we have the ENS or Ethereum Name Service, the new **[ethers.js](https://docs.ethers.io/ethers.js/html)** has
ENS names as first-class citizens and is the step in the right direction. Basically we need to create a resilient
address that can be served by anyone. This is accomplished by using hashes to guarantee that no matter who serves the
requested data, the data can be checked against the hash to ensure it's correct.

By combining a blockchain based naming service and a decentralized storage system such as [IPFS](https://ipfs.io) the concept of a decentralized
website can be achieved. However you should now understand that the internet browser cannot be used in its 
current form, unless it could perhaps accept an ENS or other blockchain based URL. Currently as blockchain is in its
infancy this is not supported but it is foreseeable that one day it could be.


### Forget websites or browsers, a DApp that directly communicates with the blockchain solves these issues

If you have followed along to this point, you should be able to intuitively understand why most DApps are mobile applications
running directly on a device. DApps directly communicate with the blockchain by design, there should not be any references to
centralized services.

However many DApps may download images, resources or libraries from traditional URLs. This does go against the spirit of 
decentralization but it's also understandable and realistic, in that we mean that being completely decentralized is idealistic and not
the highest concern for most DApp developers.

We obviously need to ensure that items of value such as tokens, unique NFTs (non-fungible tokens) or proofs need to be decentralized.
But unimportant or trivial things such as images, libraries or some content can be left to centralized services until
a later date when it may be necessary to decentralize them.  


