Instructions
===============

this is the document of ela json rpc interfaces.
it follows json-rpc 2.0 protocol but also keeps compatible with 1.0 version.
That means both named params and positional params are acceptable.

"id" is optional, which will be sent back in the result samely if you add it in a request.
It is needed when you want to distinguish different requests.

"jsonrpc" is optional. It tells which version this request uses.
In version 2.0 it is required, while in version 1.0 it does not exist.

#### getbestblockhash
description: return the hash of the most recent block

parameters: none

result:

| name      | type   | description                       |
| --------- | ------ | --------------------------------- |
| blockhash | string | the hash of the most recent block |

arguments sample:

```json
{
  "method":"getbestblockhash"
}
```

result sample:

```json
{
  "id": null,
  "jsonrpc": "2.0",
  "result": "68692d63a8bfc8887553b97f99f09e523d34a2b599bf5b388436b2ddc85ed76e",
  "error": null
}
```

#### getblockhash
description: return the hash of the specific blockchain height.

paramters:

| name   | type    | description              |
| ------ | ------- | ------------------------ |
| height | integer | the height of blockchain |

result:

| name      | type   | description           |
| --------- | ------ | --------------------- |
| blockhash | string | the hash of the block |

arguments sample:

```json
{
  "method":"getblockhash",
  "params":{"height":1}
}
```

result sample:

```json
{
  "id": null,
  "jsonrpc": "2.0",
  "result": "3893390c9fe372eab5b356a02c54d3baa41fc48918bbddfbac78cf48564d9d72",
  "error": null
}
```

#### getblock
description: return the block information of the specific blockchain hash.

parameters:

| name      | type   | description                             |
| --------- | ------ | --------------------------------------- |
| blockhash | string | the blockchain hash                     |
| verbosity | int    | the verbosity of result, can be 0, 1, 2 |

result:(verbosity=0)

raw hash

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": "00000000c0433b918f500392869aa14cf7a909430fd94502b5c9f05421c9da7519bd6a65219184ea3c0a2973b90b8402c8405b76d7fbe10a268f6de7e4f48e93f5d03df7c31e095bffff7f2000000000d107000001000000010000000000000000000000000000000000000000000000000000000000000000000000002cfabe6d6d3ca6bcc86bada4642fea709731f1653bd34b28ab15b790e102e14e0d7bd138d80100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffff7f00000000000000000000000000000000000000000000000000000000000000000ce39baabcdbb4adce38c5f23314c5f63a536bbcc8f0a47c7054c36ca27f5acd771d095b00000000020000000101000000000403454c4101000846444170b0e427d2010000000000000000000000000000000000000000000000000000000000000000ffffffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a31b2913000000000000000000129e9cf1c5f336fcf3a6c954444ed482c5d916e506b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a341b52c000000000000000000219e9cc4320c3018ced30242b25c03e13a1b2f57c7d107000000"
}
```

result:(verbosity=1)

| name              | type          | description                                                                                                                     |
| ----------------- | ------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| hash              | string        | the blockchain hash                                                                                                             |
| confirmations     | integer       | confirmations                                                                                                                   |
| size              | integer       | the size of a block in bytes                                                                                                    |
| strippedsize      | integer       | equals to size                                                                                                                  |
| weight            | integer       | This block’s weight                                                                                                             |
| height            | integer       | the height of block                                                                                                             |
| version           | integer       | block header's version                                                                                                          |
| versionhex        | string        | block header's version in hex format                                                                                            |
| merkleroot        | string        | the merkleroot hash of this block                                                                                               |
| tx                | array[string] | transaction hashes of this block, in an array                                                                                   |
| time              | integer       | the Unix timestamp of this block                                                                                                |
| mediantime        | integer       | equals to time                                                                                                                  |
| nonce             | integer       | the nonce of this block                                                                                                         |
| bits              | integer       | bits of this block                                                                                                              |
| difficulty        | string        | difficulty of this block                                                                                                        |
| chainwork         | string        | The estimated number of block header hashes miners had to check from the genesis block to this block, encoded as big-endian hex |
| previousblockhash | string        | previous block hash                                                                                                             |
| nextblockhash     | string        | next block hash                                                                                                                 |
| auxpow            | string        | Auxpow information in hex format                                                                                                |

arguments sample:

```json
{
  "method": "getblock",
  "params": ["0000000000000c128adadedd348061952fa5c9bd78320ee25052d2b74a10573f"],
  "id": 123
}
```

result sample

```json
{
  "id": null,
  "error": null,
  "jsonrpc": "2.0",
  "result": {
    "hash": "3893390c9fe372eab5b356a02c54d3baa41fc48918bbddfbac78cf48564d9d72",
    "confirmations": 5156,
    "strippedsize": 498,
    "size": 498,
    "weight": 1992,
    "height": 1,
    "version": 0,
    "versionhex": "00000000",
    "merkleroot": "764691821f937fd566bcf533611a5e5b193008ea1ba1396f67b7b0da22717c02",
    "tx": [
      "764691821f937fd566bcf533611a5e5b193008ea1ba1396f67b7b0da22717c02"
    ],
    "time": 1524737598,
    "mediantime": 1524737598,
    "nonce": 0,
    "bits": 545259519,
    "difficulty": "1",
    "chainwork": "00001423",
    "previousblockhash": "8d7014f2f941caa1972c8033b2f0a860ec8d4938b12bae2c62512852a558f405",
    "nextblockhash": "aa98305779686e66294a9b667e6ac77f5231bb2ce09fe7d9ca641775413ecb5a",
    "auxpow": "01000000010000000000000000000000000000000000000000000000000000000000000000000000002cfabe6d6d3893390c9fe372eab5b356a02c54d3baa41fc48918bbddfbac78cf48564d9d720100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffff7f0000000000000000000000000000000000000000000000000000000000000000a19035123a440356c0a41a85fe26865620fb4c34dcf1b83b46b5e11efcbbdd893ea6e15a0000000000000000"
  }
}
```

result:(verbosity=2)

result format except 'tx' is the same as it is when verbosity=1

result format in 'tx' please see interface 'getrawtransaction'

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": {
    "hash": "3ca6bcc86bada4642fea709731f1653bd34b28ab15b790e102e14e0d7bd138d8",
    "confirmations": 1,
    "strippedsize": 498,
    "size": 498,
    "weight": 1992,
    "height": 2001,
    "version": 0,
    "versionhex": "00000000",
    "merkleroot": "219184ea3c0a2973b90b8402c8405b76d7fbe10a268f6de7e4f48e93f5d03df7",
    "tx": [
      {
        "txid": "219184ea3c0a2973b90b8402c8405b76d7fbe10a268f6de7e4f48e93f5d03df7",
        "hash": "219184ea3c0a2973b90b8402c8405b76d7fbe10a268f6de7e4f48e93f5d03df7",
        "size": 192,
        "vsize": 192,
        "version": 0,
        "locktime": 2001,
        "vin": [
          {
            "txid": "0000000000000000000000000000000000000000000000000000000000000000",
            "vout": 65535,
            "sequence": 4294967295
          }
        ],
        "vout": [
          {
            "value": "0.01255707",
            "n": 0,
            "address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
            "assetid": "b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3",
            "outputlock": 0
          },
          {
            "value": "0.02929985",
            "n": 1,
            "address": "EXca4DJwqCXa6vbJmpovwatHiP8HRTVS1Z",
            "assetid": "b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3",
            "outputlock": 0
          }
        ],
        "blockhash": "3ca6bcc86bada4642fea709731f1653bd34b28ab15b790e102e14e0d7bd138d8",
        "confirmations": 1,
        "time": 1527324355,
        "blocktime": 1527324355,
        "type": 0,
        "payloadversion": 4,
        "payload": {
          "CoinbaseData": "ELA"
        },
        "attributes": [
          {
            "usage": 0,
            "data": "46444170b0e427d2"
          }
        ],
        "programs": []
      }
    ],
    "time": 1527324355,
    "mediantime": 1527324355,
    "nonce": 0,
    "bits": 545259519,
    "difficulty": "1",
    "chainwork": "00000000",
    "previousblockhash": "c0433b918f500392869aa14cf7a909430fd94502b5c9f05421c9da7519bd6a65",
    "nextblockhash": "0000000000000000000000000000000000000000000000000000000000000000",
    "auxpow": "01000000010000000000000000000000000000000000000000000000000000000000000000000000002cfabe6d6d3ca6bcc86bada4642fea709731f1653bd34b28ab15b790e102e14e0d7bd138d80100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffff7f00000000000000000000000000000000000000000000000000000000000000000ce39baabcdbb4adce38c5f23314c5f63a536bbcc8f0a47c7054c36ca27f5acd771d095b0000000002000000"
  }
}
```

#### getblockcount

description: get block count

parameters: none

argument sample:

```json
{
  "method":"getblockcount"
}
```

result sample:

```json
{
  "jsonrpc": "2.0",
  "id": null,
  "error": null,
  "result": 171454
}
```

#### getrawtransaction

description: get transaction infomation of given transaction hash.

parameters:

| name    | type   | description       |
| ------- | ------ | ----------------- |
| txid    | string | transaction hash  |
| verbose | bool   | verbose of result |

results:

| name       | type    | description                                  |
| ---------- | ------- | -------------------------------------------- |
| txid       | string  | transaction id                               |
| hash       | string  | transaction id                               |
| size       | integer | transaction size                             |
| vsize      | integer | The virtual transaction size, equals to size |
| version    | integer | The transaction format version number        |
| locktime   | integer | The transaction’s locktime                   |
| sequence   | integer | The transaction’s sequence number            |
| vin        | array   | input utxo vector of this transaction        |
| n          | integer | index of utxo outputs                        |
| vout       | array   | output utxo vector of this transaction       |
| assetid    | string  | asset id                                     |
| outputlock | string  | outputlock of this transaction               |

argument sample:

```json
{
  "method": "getrawtransaction",
  "params": ["caa0d52ea2b90a08480834b97c271a8b847aadf90057318a33ccc8674b77c796"]
}
```

result sample:(verbose=true)

```json
{
  "id": null,
  "error": null,
  "jsonrpc": "2.0",
  "result": {
    "txid": "6864bbf52a3e140d40f1d707bae31d006265efc54dcb58e34037645060ce3e16",
    "hash": "6864bbf52a3e140d40f1d707bae31d006265efc54dcb58e34037645060ce3e16",
    "size": 192,
    "vsize": 192,
    "version": 0,
    "locktime": 1000,
    "vin": [
      {
        "txid": "0000000000000000000000000000000000000000000000000000000000000000",
        "vout": 65535,
        "sequence": 4294967295
      }
    ],
    "vout": [
      {
        "value": "0.01255707",
        "n": 0,
        "address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
        "assetid": "b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3",
        "outputlock": 0
      },
      {
        "value": "0.02929985",
        "n": 1,
        "address": "ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z",
        "assetid": "b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3",
        "outputlock": 0
      }
    ],
    "blockhash": "0000000000000000000000000000000000000000000000000000000000000000",
    "confirmations": 4158,
    "time": 1524737766,
    "blocktime": 1524737766,
    "type": 0,
    "payloadversion": 4,
    "payload": {
      "CoinbaseData": "ELA"
    },
    "attributes": [
      {
        "usage": 0,
        "data": "b52165c186769037"
      }
    ],
    "programs": []
  }
}
```

result sample:(verbose=false)

return raw data

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": "000403454c4101000846444170b0e427d2010000000000000000000000000000000000000000000000000000000000000000ffffffffffff02b037db964a231458d2d  6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a31b2913000000000000000000129e9cf1c5f336fcf3a6c954444ed482c5d916e506b037db964a231458d2d6ffd  5ea18944c4f90e63d547c5d3b9874df66a4ead0a341b52c000000000000000000219e9cc4320c3018ced30242b25c03e13a1b2f57c7d107000000"
}
```

#### getrawmempool

description: return hashes of transactions in memory pool.

parameters: none

argument sample:

```json
{
  "method":"getrawmempool"
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result":["5da460632a154fe75df0d5ec98560e4bc1115374a37a75e984a534f8da3ca941", "5da460632a154fe75df0d5ec98560e4bc1115374a37a75e984a534f8da3ca941"]
}
```

#### getreceivedbyaddress
description: get the balance of an address

parameters:

| name    | type   | description |
| ------- | ------ | ----------- |
| address | string | address     |

result: the balance of the address

argument sample:

```json
{
  "method": "getreceivedbyaddress",
  "params":{"address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta"}
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": "33000000"
}
```

#### listunspent

description: list all utxo of given addresses

parameters:

| name      | type          | description   |
| --------- | ------------- | ------------- |
| addresses | array[string] | addresses     |
| utxotype  | string        | the utxo type |

if not set utxotype will use "mixed" as default value
if set utxotype to "mixed" or not set will get all utxos ignore the type
if set utxotype to "vote" will get vote utxos
if set utxotype to "normal" will get normal utxos without vote

result:
please see below

argument sample:

```json
{
  "method":"listunspent",
  "params":{"addresses": ["8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3", "EeEkSiRMZqg5rd9a2yPaWnvdPcikFtsrjE"]}
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": [
    {
      "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
      "txid": "9132cf82a18d859d200c952aec548d7895e7b654fd1761d5d059b91edbad1768",
      "vout": 0,
      "address": "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3",
      "amount": "33000000",
      "confirmations": 1102,
      "outputlock": 0
    },
    {
      "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
      "txid": "3edbcc839fd4f16c0b70869f2d477b56a006d31dc7a10d8cb49bd12628d6352e",
      "vout": 0,
      "address": "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3",
      "amount": "0.01255707",
      "confirmations": 846,
      "outputlock": 0
     }
  ]
}
```

#### setloglevel

description: set log level

parameters:

| name  | type    | description   |
| ----- | ------- | ------------- |
| level | integer | the log level |

result:
please see below

argument sample:

```json
{
  "method": "setloglevel",
  "params": {
    "level": 0
  }
}
```

result sample:

```json
{
  "id": null,
  "jsonrpc": "2.0",
  "error": null,
  "result": "log level has been set to 0"
}
```

#### getconnectioncount

description: get peer's count of this node

argument sample:

```json
{
  "method": "getconnectioncount"
}
```

result sample:

```json
{
  "id": null,
  "error": null,
  "jsonrpc": "2.0",
  "result": 0
}
```

#### getneighbors

description: get peer's info

parameters: none

argument sample:

```json
{
  "method":"getneighbors"
}
```

result sample:

```json
{
    "error": null,
    "id": 123456,
    "jsonrpc": "2.0",
    "result": [
        "127.0.0.1:64890 (inbound)",
        "127.0.0.1:64880 (inbound)",
        "127.0.0.1:64822 (inbound)",
        "127.0.0.1:22338 (outbound)",
        "127.0.0.1:23338 (outbound)",
        "127.0.0.1:24338 (outbound)"
    ]
}
```

#### getnodestate

description: get node state

parameters: none

results:

| name        | type            | description                                                 |
| ----------- | --------------- | ----------------------------------------------------------- |
| compile     | string          | node's compile version                                      |
| height      | integer         | current height of local node                                |
| version     | integer         | peer-to-peer network protocol version of this node          |
| services    | string          | the services provided by this node                          |
| port        | integer         | P2P network port                                            |
| rpcport     | integer         | JSON-RPC service port                                       |
| restport    | integer         | RESTful service port                                        |
| wsport      | integer         | webservice port                                             |
| neighbors   | array[neighbor] | neighbor nodes information                                  |

neighbor:

| name           | type    | description                                                     |
| -------------- | ------- | --------------------------------------------------------------- |
| netaddress     | string  | network address of the neighbor in host:port format             |
| services       | string  | the services neighbor provides                                  |
| relaytx        | bool    | relay transactions to the neighbor or not                       |
| lastsend       | string  | the last time send a message to the neighbor                    |
| lastrecv       | string  | the last time received a message from the neighbor              |
| conntime       | string  | the time when this neighbor was connected                       |
| timeoffset     | integer | time offset between local time and the time advertised by the neighbor |
| version        | integer | peer-to-peer network version advertised by the neighbor         |
| inbound        | bool    | the connection direction of the neighbor (inbound/outbound)     |
| startingheight | integer | the height advertised by the neighbor when connected            |
| lastblock      | integer | the height of the last block advertised by the neighbor         |
| lastpingtime   | string  | the last time send a ping message to the neighbor               |
| lastpingmicros | integer | microseconds to receive pong message after sending last ping message |

argument sample:
```json
{
  "method":"getnodestate"
}
```

```json
{
    "error": null,
    "id": 123456,
    "jsonrpc": "2.0",
    "result": {
        "compile": "v0.2.2-231-g75d2-dirty",
        "height": 0,
        "version": 20000,
        "services": "SFNodeNetwork|SFTxFiltering|SFNodeBloom",
        "port": 21338,
        "rpcport": 21336,
        "restport": 21334,
        "wsport": 21335,
        "neighbors": [
            {
                "netaddress": "127.0.0.1:57656",
                "services": "SFNodeNetwork|SFTxFiltering|SFNodeBloom",
                "relaytx": false,
                "lastsend": "2019-03-06 14:52:03 +0800 CST",
                "lastrecv": "2019-03-06 14:52:18 +0800 CST",
                "conntime": "2019-03-06 14:51:33.65298 +0800 CST m=+36.604516146",
                "timeoffset": 0,
                "version": 20000,
                "inbound": true,
                "startingheight": 0,
                "lastblock": 0,
                "lastpingtime": "2019-03-06 14:52:03.658121 +0800 CST m=+66.609840707",
                "lastpingmicros": 1033
            },
            {
                "netaddress": "127.0.0.1:22338",
                "services": "SFNodeNetwork|SFTxFiltering|SFNodeBloom",
                "relaytx": false,
                "lastsend": "2019-03-06 14:52:02 +0800 CST",
                "lastrecv": "2019-03-06 14:52:02 +0800 CST",
                "conntime": "2019-03-06 14:51:02.097149 +0800 CST m=+5.048492921",
                "timeoffset": 0,
                "version": 20000,
                "inbound": false,
                "startingheight": 0,
                "lastblock": 0,
                "lastpingtime": "2019-03-06 14:52:02.104806 +0800 CST m=+65.056516088",
                "lastpingmicros": 541
            }
        ]
    }
}
```

#### sendrawtransaction

description: send a raw transaction to node

parameters:

| name | type   | description                 |
| ---- | ------ | --------------------------- |
| data | string | raw transaction data in hex |

result:

| name | type   | description      |
| ---- | ------ | ---------------- |
| hash | string | transaction hash |

argument sample:

```json
{
  "method":"sendrawtransaction",
  "params": ["xxxxxx"]
}
```

result sample:

```json
{
  "result":"764691821f937fd566bcf533611a5e5b193008ea1ba1396f67b7b0da22717c02",
  "id": null,
  "jsonrpc": "2.0",
  "error": null
}
```

#### togglemining

description: the switch of mining

parameters:

| name   | type | description         |
| ------ | ---- | ------------------- |
| mining | bool | whether mine or not |

argument sample:

```json
{
  "method":"togglemining",
  "params":{"mining":false}
}
```

result sample:

```json
{
  "id": null,
  "jsonrpc": "2.0",
  "result": "mining stopped",
  "error": null
}
```

#### discretemining

description: generate one or more blocks instantly
parameters:

| name  | type    | description     |
| ----- | ------- | --------------- |
| count | integer | count of blocks |

argument sample:

```json
{
  "method":"discretemining",
  "params":{"count":1}
}
```

result sample:

```json
{
  "id": null,
  "jsonrpc": "2.0",
  "result": [
    "741d8131f0eea94c1c72c8bb1f0e9051a0a98441e131585bf5bf01868bf0ef46"
  ],
  "error": null
}
```

#### createauxblock

description: generate an auxiliary block
parameters:

| name         | type   | description     |
| ------------ | ------ | --------------- |
| paytoaddress | string | miner's address |

named arguments sample:

```json
{
  "method":"createauxblock",
  "params":{"paytoaddress":"Ef4UcaHwvFrFzzsyVf5YH4JBWgYgUqfTAB"}
}
```

positional arguments sample:

```json
{
  "method": "createauxblock",
  "params": ["Ef4UcaHwvFrFzzsyVf5YH4JBWgYgUqfTAB"]
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": {
    "chainid": 1224,
    "height": 152789,
    "coinbasevalue": 175799086,
    "bits": "1d36c855",
    "hash": "e28a262b38316fddefb0b5c753f7cc0022afe94e95f881576ad6b8f33f4e49fe",
    "previousblockhash": "f297d03791f4cf2c6ef093b02a77465ea876b040b7772e56b8e140f3bff73871"
  }
}
```

#### submitauxblock

description: submit the solved auxpow of an auxiliary block
parameters:

| name      | type   | description                               |
| --------- | ------ | ----------------------------------------- |
| blockhash | string | the auxiliary block hash                  |
| auxpow    | string | the solved auxpow of this auxiliary block |

named arguments sample:

```json
{
  "method":"submitauxblock",
  "params":{
    "blockhash": "7926398947f332fe534b15c628ff0cd9dc6f7d3ea59c74801dc758ac65428e64",
    "auxpow": "02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4b0313ee0904a880495b742f4254432e434f4d2ffabe6d6d9581ba0156314f1e92fd03430c6e4428a32bb3f1b9dc627102498e5cfbf26261020000004204cb9a010f32a00601000000000000ffffffff0200000000000000001976a914c0174e89bd93eacd1d5a1af4ba1802d412afc08688ac0000000000000000266a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf90000000014acac4ee8fdd8ca7e0b587b35fce8c996c70aefdf24c333038bdba7af531266000000000001ccc205f0e1cb435f50cc2f63edd53186b414fcb22b719da8c59eab066cf30bdb0000000000000020d1061d1e456cae488c063838b64c4911ce256549afadfc6a4736643359141b01551e4d94f9e8b6b03eec92bb6de1e478a0e913e5f733f5884857a7c2b965f53ca880495bffff7f20a880495b"
  }
}
```

positional arguments sample:

```json
{
  "method":"submitauxblock",
  "params":[
    "7926398947f332fe534b15c628ff0cd9dc6f7d3ea59c74801dc758ac65428e64",
    "02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4b0313ee0904a880495b742f4254432e434f4d2ffabe6d6d9581ba0156314f1e92fd03430c6e4428a32bb3f1b9dc627102498e5cfbf26261020000004204cb9a010f32a00601000000000000ffffffff0200000000000000001976a914c0174e89bd93eacd1d5a1af4ba1802d412afc08688ac0000000000000000266a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf90000000014acac4ee8fdd8ca7e0b587b35fce8c996c70aefdf24c333038bdba7af531266000000000001ccc205f0e1cb435f50cc2f63edd53186b414fcb22b719da8c59eab066cf30bdb0000000000000020d1061d1e456cae488c063838b64c4911ce256549afadfc6a4736643359141b01551e4d94f9e8b6b03eec92bb6de1e478a0e913e5f733f5884857a7c2b965f53ca880495bffff7f20a880495b"
  ]
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": true
}
```

#### getinfo

description: return node information.
warning: this interface is ready to be deprecated. So no api information will be supplied.

#### listproducers

description: show producers infromation
parameters:

| name  | type    | description                                                  |
| ----- | ------- | ------------------------------------------------------------ |
| start | integer | the start index of producers                                 |
| limit | integer | the limit count of producers                                 |
| state | string  | the producer state you want<br/>"all": get producers in any state<br/>"pending": get producers in the pendding state<br/>
"active": get producers in the active state<br/>
"inactive": get producers in the inactive state<br/>
"canceled": get producers in the canceled state<br/>
"illegal": get producers in the illegal state<br/>
"returned": get producers in the returned state |
if state flag not provided return the producers in pending and active state.

result:

| name           | type   | description                               |
| -------------- | ------ | ----------------------------------------- |
| ownerpublickey | string | the owner public key of producer          |
| nodepublickey  | string | the node public key of the producer       |
| nickname       | string | the nick name of the producer             |
| url            | string | the url of the producer                   |
| location       | uint64 | the location number of the producer       |
| active         | bool   | if producer has confirmed                 |
| votes          | string | the votes currently held                  |
| state          | string | the current state of the producer         |
| registerheight | uint32 | the height of cancel producer             |
| cancelheight   | uint32 | the cancel height of the producer         |
| inactiveheight | uint32 | the inactive start height of the producer |
| illegalheight  | uint32 | the illegal start height of the producer  |
| index          | uint64 | the index of the producer                 |
| totalvotes     | string | the total votes of registered producers   |
| totalcounts    | uint64 | the total counts of registered producers  |

named arguments sample:

```json
{
  "method": "listproducers",
  "params":{
    "start": 0,
    "limit": 3
  }
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": {
    "producers": [
      {
        "ownerpublickey": "0237a5fb316caf7587e052125585b135361be533d74b5a094a68c64c47ccd1e1eb",
        "nodepublickey": "0237a5fb316caf7587e052125585b135361be533d74b5a094a68c64c47ccd1e1eb",
        "nickname": "elastos1",
        "url": "http://www.elastos1.com",
        "location": 401,
        "active": true,
        "votes": "3.11100000",
        "state": "Active",
        "registerheight": 236,
        "cancelheight": 0,
        "inactiveheight": 0,
        "illegalheight": 0,
        "index": 0
      },
      {
        "ownerpublickey": "030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
        "nodepublickey": "030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
        "nickname": "elastos2",
        "url": "http://www.elastos2.com",
        "location": 402,
        "active": true,
        "votes": "2.10000000",
        "state": "Active",
        "registerheight": 225,
        "cancelheight": 0,
        "inactiveheight": 0,
        "illegalheight": 0,
        "index": 1
      },
      {
        "ownerpublickey": "0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
        "nodepublickey": "0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
        "nickname": "elastos3",
        "url": "http://www.elastos3.com",
        "location": 403,
        "active": true,
        "votes": "0",
        "state": "Active",
        "registerheight": 216,
        "cancelheight": 0,
        "inactiveheight": 0,
        "illegalheight": 0,
        "index": 2
      }
    ],
    "totalvotes": "5.21100000",
    "totalcounts": 10
  }
}
```

#### producerstatus

description: show producer status
parameters:

| name      | type   | description                  |
| --------- | ------ | ---------------------------- |
| publickey | string | the public key of producer   |

result:

0: producer has not registered
1: producer has confirmed (6 confirms)
2: producer registered but not confirmed (less than 6 confirms)

named arguments sample:

```json
{
  "method": "producerstatus",
  "params":{
    "publickey": "0237a5fb316caf7587e052125585b135361be533d74b5a094a68c64c47ccd1e1eb"
  }
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": 1
}
```

#### votestatus

description: show producer vote status
parameters:

| name    | type   | description         |
| ------- | ------ | ------------------- |
| address | string | the address of user |

result:

| name      | type   | description             |
| --------- | ------ | ----------------------- |
| total     | string | the total voting rights |
| voting    | string | the used voting rights  |
| pending   | bool   | have vote in tx pool    |

named arguments sample:

```json
{
  "method": "votestatus",
  "params":{
    "address": "EZwPHEMQLNBpP2VStF3gRk8EVoMM2i3hda"
  }
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": {
    "total": "4.66088900",
    "voting": "0",
    "pending": true
  }
}
```

#### estimatesmartfee

description: estimate transaction fee smartly.

parameters:

| name          | type | description                                                  |
| ------------- | ---- | ------------------------------------------------------------ |
| confirmations | int  | in how many blocks do you want your transaction to be packed |

result:

| name | type | description                       |
| ---- | ---- | --------------------------------- |
| -    | int  | fee rate, the unit is sela per KB |

named arguments sample:

```json
{
  "method": "estimatesmartfee",
  "params":{
    "confirmations": 5
  }
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": 10000
}
```

#### getdepositcoin

description: get deposit coin by owner public key.

parameters:

| name           | type   | description                    |
| -------------- | ------ | ------------------------------ |
| ownerpublickey | string | the ownerPublicKey of producer |

result:

| name      | type   | description                            |
| --------- | ------ | -------------------------------------- |
| available | string | the available deposit coin of producer |
| deducted  | string | the deducted deposit coin of producer  |

named arguments sample:

```json
{
  "method": "getdepositcoin",
  "params":{
    "ownerpublickey": "024babfecea0300971a6f0ad13b27519faff0ef595faf9490dc1f5f4d6e6d7f3fb"
  }
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": {
    "available": "3",
    "deducted": "0"
  }
}
```

#### getarbiterpeersinfo

description: get dpos peers information.

result:

| name | type | description                       |
| ---- | ---- | --------------------------------- |
| ownerpublickey | string  | owner public key of the peer which should be one of current arbiters |
| nodepublickey | string  | node public key of the peer which should be one of current arbiters |
| ip    | string  | ip address of the peer (including port) |
| connstate | string  | connection state about the peer, the value can be: NoneConnection, OutboundOnly, InboundOnly, or 2WayConnection |

named arguments sample:

```json
{
  "method": "getarbiterpeersinfo"
}
```

result sample:

```json
{
    "error": null,
    "id": null,
    "jsonrpc": "2.0",
    "result": [
        {
            "ownerpublickey": "0243ff13f1417c69686bfefc35227ad4f5f4ca03ccb3d3a635ae8ed67d57c20b97",
            "nodepublickey": "0243ff13f1417c69686bfefc35227ad4f5f4ca03ccb3d3a635ae8ed67d57c20b97",
            "ip": "127.0.0.1:22339",
            "connstate": "2WayConnection"
        },
        {
            "ownerpublickey": "024ac1cdf73e3cbe88843b2d7279e6afdc26fc71d221f28cfbecbefb2a48d48304",
            "nodepublickey": "0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
            "ip": "127.0.0.1:23339",
            "connstate": "InboundOnly"
        },
        {
            "ownerpublickey": "0274fe9f165574791f74d5c4358415596e408b704be9003f51a25e90fd527660b5",
            "nodepublickey": "03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
            "ip": "127.0.0.1:24339",
            "connstate": "NoneConnection"
        }
    ]
}
```

#### submitsidechainillegaldata

description: submit illegal data from side chain.

parameters:

| name           | type   | description                    |
| -------------- | ------ | ------------------------------ |
| illegaldata | string | serialized illegal data in hex string format |

named arguments sample:

```json
{
  "method":"submitsidechainillegaldata",
  "params":{
    "illegaldata": "016400000021023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a52fdfc072182654f163f5f0f9a621d729566c74d10037c4d7bbb0407d1e2c64981855ad8681d0d86d1e91e00167939cb6694d2c422acd208a0072939487f699940353662653933363937386332363162326536343964353864626661663366323364346138363832373466353532326364326164623433303861393535633461330221030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9210288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7"
  }
}
```
result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": true
}
```

#### getconfirmbyheight

description: get block confirm by height of block.

parameters:

| name      | type   | description                                                                 |
| --------- | ------ | --------------------------------------------------------------------------- |
| height    | int    | the height of block                                                         |
| verbosity | int    | the verbosity of result, 0 will return serialized confirmed data, default 1 |

result:

| name       | type           | description                               |
| -------    | -------------- | ----------------------------------------- |
| sponsor    | string         | the sponsor nodePublicKey of the proposal |
| viewoffset | uint32         | the viewoffset of the proposal            |
| votes      | array[struct]  | the votes of confirm                      |
| signer     | string         | the singner nodePublicKey of the proposal |
| accept     | bool           | accept or not of the proposal             |

named arguments sample:

```json
{
  "method":"getconfirmbyheight",
  "params":{
    "height": 300000,
    "verbosity": 1
  }
}
```
result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": {
    "blockhash": "65fd07f4069a1ca6833d38e0baa9b0b4a9bac35131feb5c363bd2fb99f8d06d8"
    "sponsor": "024ac1cdf73e3cbe88843b2d7279e6afdc26fc71d221f28cfbecbefb2a48d48304",
    "viewoffset": 0,
    "votes": [
      {
        "signer": "024babfecea0300971a6f0ad13b27519faff0ef595faf9490dc1f5f4d6e6d7f3fb",
        "accept": true
      },
      {
        "signer": "024ac1cdf73e3cbe88843b2d7279e6afdc26fc71d221f28cfbecbefb2a48d48304",
        "accept": true
      },
      {
        "signer": "0274fe9f165574791f74d5c4358415596e408b704be9003f51a25e90fd527660b5",
        "accept": true
      }
    ]
  }
}
```

#### getconfirmbyhash

description: get block confirm by hash of block.

parameters:

| name      | type   | description                                                                 |
| --------- | ------ | --------------------------------------------------------------------------- |
| blockhash | string | the hash of block                                                           |
| verbosity | int    | the verbosity of result, 0 will return serialized confirmed data, default 1 |
result:

| name       | type          | description                               |
| -------    | ------------- | ----------------------------------------- |
| sponsor    | string        | the sponsor nodePublicKey of the proposal               |
| viewoffset | uint32        | the viewoffset of the proposal            |
| votes      | array[struct] | the votes of confirm                      |
| signer     | string        | the singner nodePublicKey of the proposal |
| accept     | bool          | accept or not of the proposal             |

named arguments sample:

```json
{
  "method":"getconfirmbyhash",
  "params":{
    "blockhash": "65fd07f4069a1ca6833d38e0baa9b0b4a9bac35131feb5c363bd2fb99f8d06d8",
    "verbosity": 1
  }
}
```
result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": {
    "blockhash": "65fd07f4069a1ca6833d38e0baa9b0b4a9bac35131feb5c363bd2fb99f8d06d8"
    "sponsor": "024ac1cdf73e3cbe88843b2d7279e6afdc26fc71d221f28cfbecbefb2a48d48304",
    "viewoffset": 0,
    "votes": [
      {
        "signer": "024babfecea0300971a6f0ad13b27519faff0ef595faf9490dc1f5f4d6e6d7f3fb",
        "accept": true
      },
      {
        "signer": "024ac1cdf73e3cbe88843b2d7279e6afdc26fc71d221f28cfbecbefb2a48d48304",
        "accept": true
      },
      {
        "signer": "0274fe9f165574791f74d5c4358415596e408b704be9003f51a25e90fd527660b5",
        "accept": true
      }
    ]
  }
}
```
#### getarbitersinfo

description: get arbiters and candidates about current and next turn.

result:

| name | type | description                       |
| ---- | ---- | --------------------------------- |
| arbiters | array[string] | get node public keys about arbiters of current turn |
| candidates | array[string]  | get node public keys about candidates of current turn |
| nextarbiters | array[string]  | get node public keys about arbiters of next turn |
| nextcandidates | array[string]  | get node public keys about candidates of next turn |
| ondutyarbiter | string  | get node public key of current on duty arbiter |
| currentturnstartheight | integer  | get height of current turn |
| nextturnstartheight | integer  | get an estimate height of next turn |

named arguments sample:

```json
{
  "method": "getarbitersinfo"
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": {
    "arbiters": [
      "0247984879d35fe662d6dddb4edf111c9f64fde18ccf8af0a51e4b278c3411a8f2",
      "032e583b6b578cccb9bbe4a53ab54a3e3e60156c01973b16af52b614813fca1bb2",
      "0223b8e8098dd694f4d20ea74800b1260a5a4a0afe7f6a0043c7e459c84ff80fba",
        "03982eaa9744a3777860013b6b988dc5250198cb81b3aea157f9b429206e3ae80f",
      "0328443c1e4bdb5b60ec1d017056f314ba31f8f9f43806128fac20499a9df27bc2"
    ],
    "candidates": [],
    "nextarbiters": [
      "0247984879d35fe662d6dddb4edf111c9f64fde18ccf8af0a51e4b278c3411a8f2",
      "032e583b6b578cccb9bbe4a53ab54a3e3e60156c01973b16af52b614813fca1bb2",
      "0223b8e8098dd694f4d20ea74800b1260a5a4a0afe7f6a0043c7e459c84ff80fba",
      "03982eaa9744a3777860013b6b988dc5250198cb81b3aea157f9b429206e3ae80f",
      "0328443c1e4bdb5b60ec1d017056f314ba31f8f9f43806128fac20499a9df27bc2"
    ],
    "nextcandidates": [],
    "ondutyarbiter": "03982eaa9744a3777860013b6b988dc5250198cb81b3aea157f9b429206e3ae80f",
    "currentturnstartheight": 200,
    "nextturnstartheight": 212
  }
}
```

#### getutxosbyamount

description: get utxo by given amount, amount of utxo >= given amount.

parameters:

| name     | type   | description                |
| -------- | ------ | -------------------------- |
| address  | string | the address of ela         |
| amount   | string | the min amount to get utxo |
| utxotype | string | the utxo type              |

if not set utxotype will use "mixed" as default value
if set utxotype to "mixed" or not set will get all utxos ignore the type
if set utxotype to "vote" will get vote utxos
if set utxotype to "normal" will get normal utxos without vote

result:
please see below

argument sample:

```json
{
  "method":"getutxosbyamount",
  "params":{
    "address": "XKUh4GLhFJiqAMTF6HyWQrV9pK9HcGUdfJ",
    "amount": "0.25"
  }
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": [
    {
      "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
      "txid": "9132cf82a18d859d200c952aec548d7895e7b654fd1761d5d059b91edbad1768",
      "vout": 0,
      "address": "XKUh4GLhFJiqAMTF6HyWQrV9pK9HcGUdfJ",
      "amount": "0.1",
      "confirmations": 1102,
      "outputlock": 0
    },
    {
      "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
      "txid": "3edbcc839fd4f16c0b70869f2d477b56a006d31dc7a10d8cb49bd12628d6352e",
      "vout": 0,
      "address": "XKUh4GLhFJiqAMTF6HyWQrV9pK9HcGUdfJ",
      "amount": "0.2",
      "confirmations": 846,
      "outputlock": 0
     }
  ]
}
```

#### getamountbyinputs

description: get amount of given inputs.

parameters:

| name    | type   | description              |
| ------- | ------ | ------------------------ |
| inputs  | string | the hex string of inputs |

result:
amount of all given inputs, the type is string, if not found input will return error

argument sample:

```json
{
  "method":"getamountbyinputs",
  "params":{
    "inputs": "029132cf82a18d859d200c952aec548d7895e7b654fd1761d5d059b91edbad17680000000000003edbcc839fd4f16c0b70869f2d477b56a006d31dc7a10d8cb49bd12628d6352e000000000000"
  }
}
```

result sample:

```json
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": "0.3"
}
```

#### getblockbyheight

description: get a block by specifying block height.

parameters:

| name   | type   | description         |
| ------ | ------ | ------------------- |
| height | uint32 | the height of block |

result:

| name              | type          | description                                                  |
| ----------------- | ------------- | ------------------------------------------------------------ |
| hash              | string        | the blockchain hash                                          |
| confirmations     | integer       | confirmations                                                |
| size              | integer       | the size of a block in bytes                                 |
| strippedsize      | integer       | equals to size                                               |
| weight            | integer       | This block’s weight                                          |
| height            | integer       | the height of block                                          |
| version           | integer       | block header's version                                       |
| versionhex        | string        | block header's version in hex format                         |
| merkleroot        | string        | the merkleroot hash of this block                            |
| tx                | array[struct] | transactions of this block as an array                       |
| time              | integer       | the Unix timestamp of this block                             |
| mediantime        | integer       | equals to time                                               |
| nonce             | integer       | the nonce of this block                                      |
| bits              | integer       | bits of this block                                           |
| difficulty        | string        | difficulty of this block                                     |
| chainwork         | string        | The estimated number of block header hashes miners had to check from the genesis block to this block, encoded as big-endian hex |
| previousblockhash | string        | previous block hash                                          |
| nextblockhash     | string        | next block hash                                              |
| auxpow            | string        | Auxpow information in hex format                             |

argument sample:

```json
{
  "method":"getblockbyheight",
  "params":{
    "height":"5"
  }
}
```

result sample:

```json
{
    "error": null,
    "id": null,
    "jsonrpc": "2.0",
    "result": {
        "hash": "d428cf8a8e8e2c265ccceb0ed0a017aae45a89c98529d1f6dd43efc219089e0e",
        "confirmations": 452,
        "strippedsize": 563,
        "size": 563,
        "weight": 2252,
        "height": 5,
        "version": 0,
        "versionhex": "00000000",
        "merkleroot": "c1accbe3434279f74db674728d7020190680474f11c4ec2976bd1223cf7c9c66",
        "tx": [
            {
                "txid": "c1accbe3434279f74db674728d7020190680474f11c4ec2976bd1223cf7c9c66",
                "hash": "c1accbe3434279f74db674728d7020190680474f11c4ec2976bd1223cf7c9c66",
                "size": 257,
                "vsize": 257,
                "version": 0,
                "locktime": 5,
                "vin": [
                    {
                        "txid": "0000000000000000000000000000000000000000000000000000000000000000",
                        "vout": 65535,
                        "sequence": 4294967295
                    }
                ],
                "vout": [
                    {
                        "value": "1.50684931",
                        "n": 0,
                        "address": "EPha6MJ2Y9HAdrtNvMVrBu6ePMnmZJtmyv",
                        "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                        "outputlock": 0,
                        "type": 0,
                        "payload": null
                    },
                    {
                        "value": "1.75799086",
                        "n": 1,
                        "address": "EQzEvQbz5XGDZeWu2u48dPaeLTzvmfSyG4",
                        "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                        "outputlock": 0,
                        "type": 0,
                        "payload": null
                    },
                    {
                        "value": "1.75799088",
                        "n": 2,
                        "address": "EPha6MJ2Y9HAdrtNvMVrBu6ePMnmZJtmyv",
                        "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                        "outputlock": 0,
                        "type": 0,
                        "payload": null
                    }
                ],
                "blockhash": "d428cf8a8e8e2c265ccceb0ed0a017aae45a89c98529d1f6dd43efc219089e0e",
                "confirmations": 452,
                "time": 1556595214,
                "blocktime": 1556595214,
                "type": 0,
                "payloadversion": 4,
                "payload": {
                    "coinbasedata": "ELA"
                },
                "attributes": [
                    {
                        "usage": 0,
                        "data": "f53c107eec00d337"
                    }
                ],
                "programs": []
            }
        ],
        "time": 1556595214,
        "mediantime": 1556595214,
        "nonce": 0,
        "bits": 545259519,
        "difficulty": "1",
        "chainwork": "000001c3",
        "previousblockhash": "b45f9e479340c6d2889076f6bd1e138d14e8620bc2cc0ef642bd15278509d49f",
        "nextblockhash": "185cf0322f3f38abe0ecc0fffbca84d87965492042e87036bf78bfdb665b53fa",
        "auxpow": "01000000010000000000000000000000000000000000000000000000000000000000000000000000002cfabe6d6d0e9e0819c2ef43ddf6d12985c9895ae4aa17a0d00eebcc5c262c8e8e8acf28d40100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffff7f0000000000000000000000000000000000000000000000000000000000000000a0c69cab73b8541e6d5b17d5c27e3482d3cc0264da80142645f4ad67677a44940cc2c75c0000000001000000",
        "minerinfo": "ELA"
    }
}
```

#### getarbitratorgroupbyheight

description: get amount of given inputs.

parameters:

| name   | type   | description                  |
| ------ | ------ | ---------------------------- |
| height | uint32 | block height about the chain |

result:

| name                  | type          | description                         |
| --------------------- | ------------- | ----------------------------------- |
| ondutyarbitratorindex | int           | index of current on duty arbitrator |
| arbitrators           | array[string] | an array of current arbitrators     |

argument sample:

```json
{
  "method":"getarbitratorgroupbyheight",
  "params":{
    "height":"310"
  }
}
```

result sample:

```json
{
    "error": null,
    "id": null,
    "jsonrpc": "2.0",
    "result": {
        "ondutyarbitratorindex": 10,
        "arbitrators": [
            "02338fc098e08ed9a798f0d40b5320f52ad0539b98a972856a948bf652b0014110",
            "026ef6740c405e9ff137410f47fe92597c82a8dd236e87e7f4aafe4dc1aa0cd06b",
            "02c684ec9883bb5397243d9e129b9334a643f4778c28e99d59858162884081b183",
            "02d20a48b4287737912de6af53b6afef2596a755f2a541374dda1fd3ab0aa1b984",
            "02d2dd22fa2abfebb94d34666da47b15f214cc1a9ea1b8db0fda22d264bb922b36",
            "02fc936192cfb02b8a22e4ba090638f05ac60c8bf9d70f1558c5166cb7768a182e",
            "030936942e4bbe3d7f7dc45cea38195f4c4bb1474b336d6bf35fcba179872332b4",
            "0320cb8d823c5d202e0ec481f4e0fa0d3e191fb9d0a7629543882bad7444ad3016",
            "03268eb4064889047863485288515cdd880a986b57ccc9d72ae58d4401f67509a7",
            "034282d36e034272f7c289fd9655797a352d1c76a917e94d6dedaf484acf167e2d",
            "03d732f3df7c081d149bd585b77afe1077cedb4b6c6fd9b6278a8ffd456d34c3ab",
            "03f9a1f5b23e3d57e8d95a45008ad4574352fa55f755a655c81ee22e62894811b2"
        ]
    }
}
```

#### getexistwithdrawtransactions

description: find out which are already exist in chain by providing a list of  withdraw transaction hashes.

parameters:

| name | type          | description                                   |
| ---- | ------------- | --------------------------------------------- |
| txs  | array[string] | a list of transaction hashes in string format |

result:

a list of existing transaction hashes

argument sample:

```json
{
  "method":"getexistwithdrawtransactions",
  "params":{
    "txs":[
      "3edbcc839fd4f16c0b70869f2d477b56a006d31dc7a10d8cb49bd12628d6352e",
      "9132cf82a18d859d200c952aec548d7895e7b654fd1761d5d059b91edbad1768",
      "764691821f937fd566bcf533611a5e5b193008ea1ba1396f67b7b0da22717c02"
    ]
  }
}
```

result sample:

```json
{
    "error": null,
    "id": null,
    "jsonrpc": "2.0",
    "result": [
      "3edbcc839fd4f16c0b70869f2d477b56a006d31dc7a10d8cb49bd12628d6352e",
      "9132cf82a18d859d200c952aec548d7895e7b654fd1761d5d059b91edbad1768"
    ]
}
```

####