
# ELA Mainchain

?> This can be found at [https://github.com/elastos/Elastos.ELA](https://github.com/elastos/Elastos.ELA)<br/>- use tag `v0.2.2`

#### Follow the instructions in the `README.md`

But here's some key points about the `config.json` to keep in mind -&nbsp;[more info here](/core/mainchain/config.md)

- You can connect to any network by knowing the `SeedList` and `MagicNumber`, each network has a different `MagicNumber` which serves as an unique identifier.
- `ActiveNet` only governs the behavior of the ELA node, which network you connect to is determined by the `SeedList` and `MagicNumber`. You only have two options here,
- The options for `ActiveNet` (networks) are currently MainNet, TestNet (actually Elastos DevNet), GMUTestNet (Coming Soon) and RegNet (Private Core Team Only)
- `PowConfiguration.MinerInfo` can be left as "ELA"

#### Once it's running you can verify it's working by calling the Restful API on the `HttpRestPort` port defined in `config.json`.

*ELA convention is that this is the port ending in **334*

The API endpoints are defined in [https://github.com/elastos/Elastos.ELA/blob/release_v0.2.2/docs/Restful_API.md](https://github.com/elastos/Elastos.ELA/blob/release_v0.2.2/docs/Restful_API.md)


### Errors

```
[WRN] GID 43, [CheckTransactionContext] duplicate transaction check failed.
[WRN] GID 43, check transaction context failed, wrong transaction: 22ea2528a741611b846c9c67105712486c7ff79bb46bc8cc6d13a5226923d8cd
```

On a mainchain to sidechain transfer this occurs, doesn't seem to have an impact.
