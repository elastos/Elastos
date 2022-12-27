
# Setup Steps

Locally the expected setup is having

- 1 ELA Mainchain node
- 1 DID Sidechain node
- 2 Arbiters - this facilitates transactions between the mainchain and the sidechains (mininum is 2)

!> Please reference the Git Branch/SHA list at [/privnet/sha-list.md](/privnet/sha-list.md) - this lists the branch and sha you should be using for each component

To set this up follow the steps throughout this documentation site for each component, the assumed order of
setup is as follows:

### 1. Build the Elastos.ELA.Client

[Build Guide is Here](/tools/ela-client.md)

We need this to generate our foundation address and miner address. This is where the initial 33 million in the genesis block and mining rewards will go respectively.

Now call `./ela-cli wallet -c -p elastos -n keystore-priv-foundation.dat` and it will spit out a new wallet address and public key for you to use.
The private key will be encrypted by the password defined by -p (in this case it's "elastos") and inside the keystore file.

Then make a miner address: `./ela-cli wallet -c -p elastos -n keystore-priv-miner.dat`

!> Note: the `ela-cli` built by `github.com/elastos/Elastos.ELA` is currently incompatible with `Elastos.ELA.Arbiter`, so don't use that one.

Review the [Elastos.ELA.Client](/tools/ela-client.md) documentation for more functions such as retrieving your private key.

<details>
<summary>Verify the addresses are in the <code>wallet.db</code></summary>

<code>
INDEX                            ADDRESS BALANCE                           (LOCKED)   TYPE
----- ---------------------------------- ------------------------------------------ ------
    1 ERfSYxiBDs1pjrUQjN5hHTdGYGeFkNJVTf 0                                      (0) MASTER
----- ---------------------------------- ------------------------------------------ ------
    2 EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE 0                                      (0) MASTER
----- ---------------------------------- ------------------------------------------ ------
</code>

</details>


### 2. Setup the ELA Mainchain seed node

[Guide is here](/core/mainchain/mainchain.md)

But since we are setting up a new genesis block we need to do the following things:

1. Delete the `glide.lock` and `vendor` folder if it exists, then run `glide cc && glide update && glide install` make sure you delete the `elastos` and `Chain` folder in case it was leftover

2. change the genesis block timestamp in `blockchain/blockchain.go - line 106` to near the current time

    <details>
    <summary>Why do we do this? Click Here</summary>

    This has to do with the block time algorithm, every 720 blocks the difficulty is changed to keep the block time approx 2 mins.
    However every 24 hours of inactivity the difficulty decreases, so if we don't change the genesis time our block time will never recover to a nominal interval.
    </details>

3. call `make` to build the `ela` executable

4. setup `config.json`

5. start the ela node - `./ela`. For now I recommend running it in the foreground so you can visualize it, but you can run it in the background with `nohup ./ela 2>log.out 1>/dev/null &`.
For more details on this see [Tips & Tricks](/privnet/tips.md)

6. Verify the mainchain height is increasing `http://localhost:20334/api/v1/block/height`

7. Verify the wallet addresses have the expected ELA

    <details>
    <summary>Show example</summary>

    Index 1 - Foundation Address
    Index 2 - Miner Address

    <code>
    INDEX                            ADDRESS BALANCE                           (LOCKED)   TYPE
    ----- ---------------------------------- ------------------------------------------ ------
        1 ERfSYxiBDs1pjrUQjN5hHTdGYGeFkNJVTf 33000297.10045729           (326.48401900) MASTER
    ----- ---------------------------------- ------------------------------------------ ------
        2 EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE 159.97716826                (175.79908600) MASTER
    ----- ---------------------------------- ------------------------------------------ ------
    </code>
    </details>

!> Note: You cannot access any mined ELA until 100 confirmations have passed

##### Some Key Config Options:

- `SeedList` just set this to your expected mainchain nodes, don't worry about loopback
- `FoundationAddress` is where all the genesis tokens go, this is where you can put your `foundation address` from step 1.
- `PowConfiguration.PayToAddr` is the `miner address`


<details>
<summary>Example <code>config.json</code></summary>

<code>
{
  "Configuration": {
    "Magic": 7638401,
    "Version": 0,
    "SeedList": [
      "127.0.0.1:20338",
      "127.0.0.1:21338"
    ],
    "HttpInfoPort": 20333,
    "HttpInfoStart": true,
    "HttpRestPort": 20334,
    "HttpWsPort": 20335,
    "WsHeartbeatInterval": 60,
    "HttpJsonPort": 20336,
    "NodePort": 20338,
    "NodeOpenPort": 20866,
    "OpenService": true,
    "PrintLevel": 0,
    "MaxLogsSize": 0,
    "MaxPerLogSize": 0,
    "IsTLS": false,
    "CertPath": "./sample-cert.pem",
    "KeyPath": "./sample-cert-key.pem",
    "CAPath": "./sample-ca.pem",
    "FoundationAddress": "ERfSYxiBDs1pjrUQjN5hHTdGYGeFkNJVTf",
    "MultiCoreNum": 1,
    "MaxTransactionInBlock": 10000,
    "MaxBlockSize": 8000000,
    "MinCrossChainTxFee": 10000,
    "PowConfiguration": {
      "PayToAddr": "EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE",
      "AutoMining": true,
      "MinerInfo": "ELA",
      "MinTxFee": 100,
      "ActiveNet": "MainNet"
    },
    "VoteHeight": 100,
    "RpcConfiguration": {
      "User": "",
      "Pass": "",
      "WhiteIPList": [
        "0.0.0.0"
      ]
    },
    "Arbiters": [
      "023d24fa56c85c8bb28a141c98ff8e4da08c8efafe9b408903a81b020703d902d9",
      "032b62ee685897c143f01b44d273d21ee7a4a988831f73b216f0124ef83acc30f0"
    ]
  }
}
</code>

</details>


### 3. Setup the DID Sidechain Nodes

[Build Guide is Here](/core/sidechain-did/did.md)

<details>
<summary>Example <code>config.json</code></summary>

<code>
{
  "Configuration": {
    "Magic": 7638402,
    "SpvMagic": 7638401,
    "Version": 23,
    "SeedList": [
      "127.0.0.1:20608",
      "127.0.0.1:21608"
    ],
    "SpvSeedList": [
      "127.0.0.1:20866",
      "127.0.0.1:21866"
    ],
    "ExchangeRate": 1.0,
    "MinCrossChainTxFee": 10000,
    "HttpRestPort": 20604,
    "HttpWsPort": 20605,
    "HttpJsonPort": 20606,
    "NodePort": 20608,
    "PrintLevel": 1,
    "MaxLogsSize": 0,
    "MaxPerLogSize": 0,
    "DisableTxFilters": false,
    "MainChainFoundationAddress": "ERfSYxiBDs1pjrUQjN5hHTdGYGeFkNJVTf",
    "FoundationAddress": "ERfSYxiBDs1pjrUQjN5hHTdGYGeFkNJVTf",
    "PowConfiguration": {
      "PayToAddr": "EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE",
      "AutoMining": true,
      "MinerInfo": "DID",
      "MinTxFee": 100,
      "InstantBlock": false
    }
  }
}
</code>

</details>


### 5. Setup the Arbiters

[Guide is Here](/core/arbiter/arbiter.md)


### 6. Setup the Wallet Service

[Guide is Here](/services/wallet.md)

We will use this to as the API to work with your mainchain, let's try creating a wallet:

Just call `curl localhost:8091/api/1/createWallet` and it will return your new wallet info, make sure you jot this down.

If you had an app you can imagine that this would be a simple HTTP call by your back-end.

#### Now try sending some ELA from your miner address to this new address.

*TODO*

#### More importantly we need to send ELA from the Mainchain to the Sidechain



