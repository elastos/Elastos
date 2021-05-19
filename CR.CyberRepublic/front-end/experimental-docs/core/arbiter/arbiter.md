
# ELA Arbiter

The arbiter is also known as the dPoS nodes, they handle the cross-chain transactions between the mainchain and the sidechains.

!> This is currently a Private Repo - [https://github.com/elastos/Elastos.ELA.Arbiter](https://github.com/elastos/Elastos.ELA.Arbiter)

### Keystore Creation

The first thing you need to do is create `keystore.dat` files based on the number of arbiter nodes and sidechains.

?> Each arbiter node needs a primary `keystore.dat` (filename must be exactly that) and another `keystore-x.dat` for each sidechain (this can be named anything).

**Steps:**

1. Copy `ela-cli` executable from `Elastos.ELA.Client`

2. Assuming we are just doing one sidechain, you first create the primary keystore:

`./ela-cli wallet -c -p elastos -n keystore.dat`

then export the private key

`./ela-cli wallet -p elastos -n keystore.dat --export`

Do the same thing for the DID Sidechain keystore:

```
./ela-cli wallet -c -p elastos -n keystore-did.dat
./ela-cli wallet -p elastos -n keystore-did.dat --export
```

#### Important Notes:

- USE THE SAME PASSWORD (defined by the `-p` flag above) FOR BOTH KEYSTORES for each Arbiter
- You need to have at least 2 Arbiters


!> Write down your address, public and private keys.



### Setup

1. Delete `glide.lock`, `vendor` folder and install dependencies `glide cc && glide update && glide install`

2. Delete the `elastos_arbiter` folder if it exists so we can start new

3. Set the `config.json` fields

    - the `MainNode` is the connection to `Elastos.ELA`, so `SpvSeedList` should correspond to the IP and `NodeOpenPort`
    - You need a separate SideNode in `SideNodeList` for each side chain
    - Each side chain's `GenesisBlock` needs to be retrived via something like `http://localhost:20604/api/v1/block/hash/0`, make sure you use the right port defined by `HttpRestPort`.

4. Add the public keys from each of the Arbiter's primary keystore.dat to the `Arbiters` array in the `Elastos.ELA` mainchain `config.json`

5. Now run the arbiter, make sure you pass the password for the keystores, there is only one because your keystores should be using the same password, e.g. if your password is elastos - `./arbiter -p elastos`


### Errors

```
[ERR] GID 1960, [sideChainPowTransfer] create aux block failed: Unknown Block
[WRN] GID 1960, Unknown Block
```

At the moment I am seeing this too and do not know the issue, however it doesn't seem to impact normal usage.


```
2019/02/20 10:03:52.994704 [INF] GID 225, [OnDutyArbitratorChanged] I am on duty of main
2019/02/20 10:03:52.994804 [INF] GID 225, [SyncMainChainCachedTxs] start
2019/02/20 10:03:52.995086 [INF] GID 225, [SyncMainChainCachedTxs] end
2019/02/20 10:03:52.995195 [WRN] GID 225, [SyncMainChainCachedTxs] No main chain tx in dbcache
2019/02/20 10:03:52.995281 [INF] GID 3284, [OnDutyChanged] Start side chain mining: genesis address [ XKUh4GLhFJiqAMTF6HyWQrV9pK9HcGUdfJ ]
2019/02/20 10:03:52.995304 [INF] GID 3283, [SendCachedWithdrawTxs] start
2019/02/20 10:03:52.995367 [INF] GID 3284, [sideChainPowTransfer] start
2019/02/20 10:03:52.995644 [INF] GID 3283, No cached withdraw transaction need to send
2019/02/20 10:03:52.995749 [INF] GID 3283, [SendCachedWithdrawTxs] end
2019/02/20 10:03:52.996358 [INF] GID 3284, sideGenesisHash: a3c455a90843db2acd22554f2768a8d4233fafbf8dd549e6b261c2786993be56 sideBlockHash: 6563bcfbf5c5bf3c195c89067839f571612f6e52568513756a52da9cc19b1260
2019/02/20 10:03:52.997157 [WRN] GID 3284, [sideChainPowTransfer] create transaction failed: [Wallet], Available token is not enough
```

