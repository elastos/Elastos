
# Getting Started

Your personal private net is a locally running Core Elastos Ecosystem.

If you don't have access to private Elastos repos this won't work for you, in that case you will
have to rely on the docker container.


## Setting Up Your PrivateNet (PrivNet)

**This includes:**

- ELA Mainchain
- ELA Sidechain (DID)
- ELA Sidechain (ETH)
- ELA Arbiter Nodes (dPoS)
- IPFS
- PeerPad


## Local Environment or Docker Container

?> You can either set everything up locally or download our docker image which has everything up and running already.

### Setting Up Your Own Environment (OSX)

##### Required Dependencies

- Go@1.9 - `brew install go@1.9`

##### Directory Structure

We would like to keep everything in one root, this will all be relative to your `$HOME` variable

These should be set in your `~/.bash_profile`, don't forget to call it after the edits `source ~/.bash_profile`


### Steps

Locally the recommended setup is having

- 2 ELA Mainchain nodes - 1 dormant as a seed and 1 mining
- 2 DID Sidechain nodes - 1 dormant as a seed and 1 mining
- 1 Arbiter - this facilitates transactions between the mainchain and the sidechains

!> Please reference the Git Branch/SHA list at [/privnet/sha-list.md](/privnet/sha-list.md) - this lists the branch and sha you should be using for each component

To set this up follow the steps throughout this documentation site for each component, the assumed order of
setup is as follows:

#### 1. Build the Elastos.ELA.Client -&nbsp;[

We need this to generate our foundation address and private key, this is where the initial 33 million in the genesis block will go



#### 2. Setup the ELA Mainchain seed node -&nbsp;[guide is here](/core/mainchain/mainchain.md), don't have it mine by setting the `PowConfiguration` in `config.json` to:

```
"PowConfiguration": {
  "PayToAddr": "",
  "AutoMining": false,
  "MinerInfo": "local",
  "MinTxFee": 100,
  "ActiveNet": "MainNet"
},
```

- `SeedList` should be empty since this is your first node
- `FoundationAddress` is where all the genesis tokens go, if you need access to that create a new wallet address where you have private key
- `PowConfiguration.PayToAddr` is empty because we are not mining with this node.


#### 3. Setup the Wallet Service -&nbsp;[guide is here](/services/wallet.md)

We will use this service to make your first wallet on your new chain,

Just call `curl localhost:<>/api/1/createWallet` and it will return your new wallet info, make sure you jot this down.

#### 3. Setup the secondary ELA node that will do the mining

Put your newly created wallet address in the `PayToAddr` field and watch the ELA roll in!

```
"PowConfiguration": {
  "PayToAddr": "ESkBFi96dJb5RHoJDhM3EqBSKnkRuEfKjR",
  "AutoMining": true,
  "MinerInfo": "local2",
  "MinTxFee": 100,
  "ActiveNet": "MainNet"
},
```
