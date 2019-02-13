
# DID Sidechain

### Requirements

### Configuration

Copy the `sample.json` from docs folder

### Setup

1. Update `config.json`
    - Set the magic number `Magic` to match your mainchain magic number
    - Set a new spv magic number `SpvMagic`
    - Set both the `MainChainFoundationAddress` and the `FoundationAddress` field to be the same and matching your the `FoundationAddress` in your mainchain `config.json`
    - `SeedList` refers to other DID Sidechain nodes, if this is your first seed node you can leave this field out

2. Delete folder `elastos_did` if it exists, we are starting anew, if you have a previous install from a different version, you may need to run `glide cc` too.

3. Follow the **Build the node** instructions in the `README.md`
