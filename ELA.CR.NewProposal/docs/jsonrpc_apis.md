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

| name   | type | description |
| ------ | ---- | ----------- |
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

| name | type | description |
| ---- | ---- | ----------- |
| height | integer | the height of blockchain |

result:

| name   | type | description |
| ------ | ---- | ----------- |
| blockhash | string | the hash of the block | 

arguments sample:
```json
{
	"method":"getblockhash",
	"params":{"height":1}
}
```
result sample:
```javascript
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

| name | type | description |
| ---- | ---- | ----------- |
| blockhash | string | the blockchain hash | 

result:

| name | type | description |
| ---- | ---- | ----------- |
| hash | string | the blockchain hash |
| confirmations | integer | confirmations |
| size | integer | the size of a block in bytes |
| strippedsize | integer | equals to size |
| weight | integer | This block’s weight |
| height | integer | the height of block |
| version | integer | block header's version |
| versionhex | string | block header's version in hex format |
| merkleroot | string | the merkleroot hash of this block |
| tx | array[string] | transaction hashes of this block, in an array|
| time | integer | the Unix timestamp of this block |
| mediantime | integer | equals to time |
| nonce | integer | the nonce of this block |
| bits | integer | bits of this block |
| difficulty | string | difficulty of this block |
| chainwork | string | The estimated number of block header hashes miners had to check from the genesis block to this block, encoded as big-endian hex |
| previousblockhash | string | previous block hash |
| nextblockhash | string | next block hash |
| auxpow | string | Auxpow information in hex format |

arguments sample:
```javascript
{
  "method":"getblock",
  "params":["0000000000000c128adadedd348061952fa5c9bd78320ee25052d2b74a10573f"],
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

#### getblockcount

description: get block count

parameters: none

argument sample:
```javascript
{   
  "method":"getblockcount"
}
```
result sample:
```javascript
{
    "result": 171454
    "id": null,
    "error": null,
    "jsonrpc": "2.0",
}
```

#### getrawtransaction

description: get transaction infomation of given transaction hash.

parameters:

| name | type | description |
| ---- | ---- | ----------- |
| txid | string | transaction hash |

results:

| name | type | description |
| ---- | ---- | ----------- |
| txid | string | transaction id |
| hash | string | transaction id |
| size | integer | transaction size |
| vsize | integer | The virtual transaction size, equals to size || version | integer | The transaction format version number |
| locktime | integer | The transaction’s locktime |
| sequence | integer | The transaction’s sequence number |
| vin | array | input utxo vector of this transaction |
| n   | integer | index of utxo outputs |
| vout | array | output utxo vector of this transaction |
| assetid | string | asset id |
| outputlock | string | outputlock of this transaction |

argument sample:
```javascript
{
	"method":"getrawtransaction",
	"params":["caa0d52ea2b90a08480834b97c271a8b847aadf90057318a33ccc8674b77c796"]
}
```
result sample:
```javascript
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

#### getrawmempool

description: return hashes of transactions in memory pool.

parameters: none

argument sample:
```javascript
{
  "method":"getrawmempool"
}
```

result sample:

```javascript
{
  "result":["5da460632a154fe75df0d5ec98560e4bc1115374a37a75e984a534f8da3ca941", "5da460632a154fe75df0d5ec98560e4bc1115374a37a75e984a534f8da3ca941"]
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
}
```
#### setloglevel

description: set log level

parameters: 

| name | type | description |
| ---- | ---- | ----------- |
| level | integer | the log level |

result:
please see below

argument sample:
```json
{
	"method":"setloglevel",
	"params":{
		"level":0
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
```javascript
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

results:

| name | type | description |
| ---- | ---- | ----------- |
| Time | integer | current time in unix nano format |
| Services | integer | node service type. 4 is spv service and 0 is no spv service |
| IP | arrry[integer] | ip in 16-byte representation |
| Port | integer | p2p network port |
| ID | integer | node's id | 

argument sample:
```json
{
  "method":"getneighbors"
}
```
result sample:
```javascript
{
    "id": null,
    "error": null,
    "jsonrpc": "2.0",
    "result": [
        {
            "Time": 1524798750979702000,
            "Services": 4,
            "IP": [0,0,0,0,0,0,0,0,0,0,255,255,127,0,0,1],
            "Port": 30338,
            "ID": 8775829619427993046
        }
    ]
}
```

#### getnodestate

description: get node state

parameters: none

results:

| name | type | description |
| ---- | ---- | ----------- |
| Time | integer | current time in unix nano format |
| State | integer | node's state | 
| Version | integer | node's version in config.json |
| Services | integer | node service type. 4 is spv service and 0 is no spv service |
| IP | arrry[integer] | ip in 16-byte representation |
| Relay | bool | whether node will relay transaction or not |
| TxnCnt | integer | transactions transmitted by this node |
| RxTxnCnt | integer | The transaction received by this node |
| Height | integer | current height |
| Port | integer | p2p network port |
| ID | integer | node's id | 

argument sample:
```json
{
  "method":"getnodestate"
} 
```

```json
{
    "id": null,
    "error": null,
    "jsonrpc": "2.0",
    "result": {
        "State": 0,
        "Port": 20338,
        "ID": 16645880157291893421,
        "Time": 1524198859097844000,
        "Version": 1,
        "Services": 4,
        "Relay": true,
        "Height": 0,
        "TxnCnt": 0,
        "RxTxnCnt": 0
    }
}
```

#### sendrawtransaction

description: send a raw transaction to node

parameters: 

| name | type | description |
| ---- | ---- | ----------- |
| data | string | raw transaction data in hex |

result:

| name | type | description |
| ---- | ---- | ----------- |
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

| name | type | description |
| ---- | ---- | ----------- |
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
}
```

#### discretemining

description: generate one or more blocks instantly  
parameters:

| name | type | description |
| ---- | ---- | ----------- |
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
}
```
#### getinfo

description: return node information.  
warning: this interface is ready to be deprecated. So no api information will be supplied.
