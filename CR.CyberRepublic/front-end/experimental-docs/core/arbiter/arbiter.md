
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

1. Install dependencies `glide cc && glide update && glide install`

2. Delete the `elastos_arbiter` folder if it exists so we can start new

3. Set the `config.json` fields

    - the `MainNode` is the connection to `Elastos.ELA`, so `SpvSeedList` should correspond to the IP and `NodeOpenPort`
    - You need a separate SideNode in `SideNodeList` for each side chain
    - Each side chain's `GenesisBlock` needs to be retrived via something like `http://localhost:20604/api/v1/block/hash/0`, make sure you use the right port defined by `HttpRestPort`.

4. Add the public keys from each of the Arbiter's primary keystore.dat to the `Arbiters` array in the `Elastos.ELA` mainchain `config.json`

5. Now run the arbiter, make sure you pass the password for the keystores, there is only one because your keystores should be using the same password, e.g. if your password is elastos - `./arbiter -p elastos`
