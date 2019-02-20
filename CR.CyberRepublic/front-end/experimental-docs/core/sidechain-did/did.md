
# DID Sidechain

?> This can be found at [https://github.com/elastos/Elastos.ELA.SideChain.ID](https://github.com/elastos/Elastos.ELA.Sidechain.ID)<br/>- use tag `v0.0.2`

### Configuration

Copy the `sample.json` from docs folder and rename it to `config.json`

### Setup

1. Update `config.json`
    - Set the magic number `Magic` to match your mainchain magic number
    - Set a new spv magic number `SpvMagic`, this can be anything
    - Set both the `MainChainFoundationAddress` and the `FoundationAddress` field to be the same and matching your the `FoundationAddress` in your mainchain `config.json`
    - `SeedList` refers to other DID Sidechain nodes
    - `SpvSeedList` refers to your Mainchain node's IP and `NodeOpenPort` config
    - `PowConfiguration.PayToAddr` can be any address created from `Elastos.ELA.Client`

2. Delete folder `elastos_did` if it exists, we are starting anew, if you have a previous install from a different version, you may need to run `glide cc` too.

3. Follow the **Build the node** instructions in the `README.md`

