
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

