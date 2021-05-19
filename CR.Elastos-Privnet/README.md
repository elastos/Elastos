# Elastos Local PrivateNet - v0.6

This version supports the entirety of AuxPoW+DPoS(2 CRC supernodes and 2 DPoS supernodes)

### This is official but unsupported and ALPHA version

These executables are built from repos at [https://github.com/elastos](https://github.com/elastos)
which we use to compile the executables into the right environment. Only the executables can be found here.

#### These executables are purpose built for the docker images herein, if you want to use them directly, you may need to build from source

## Building from Source

Basically we don't advise this and although most repos are public, some are still private as we do internal security audits.
Therefore, you wouldn't be able to build it anyway unless you are a partner and request access.

Here's a taste of how this build process looks from scratch [https://www.cyberrepublic.org/experimental-docs](https://www.cyberrepublic.org/experimental-docs) 

## Requirements

- Ubuntu 18 or higher
- Docker Engine 18.09.2
- Docker Compose 1.23.2
- Docker Machine 0.16.1

## What you will find on Elastos PrivNet

- [Elastos Blockchain - Main chain, Arbitrator, DID Sidechain, Token Sidechain](./blockchain)
- [Elastos Hive - Decentralized and Distributed Storage](./hive)
- [Elastos Carrier - Decentralized peer to peer end-to-end encrypted network](./carrier)
