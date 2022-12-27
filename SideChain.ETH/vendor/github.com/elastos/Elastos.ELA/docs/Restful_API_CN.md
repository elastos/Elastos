# The Node API Of Elastos

`ELA Node` 使用 `2*334` 端口提供如下接口服务：

* `/api/v1/node/connectioncount` : 获取当前节点所连接到的节点数量

   示例：

    ```bash
    curl http://localhost:20334/api/v1/node/connectioncount
    {
        "Desc": "Success",
        "Error": 0,
        "Result": 5
    }
    ```

* `/api/v1/node/state` : 获取当前节点状态

   示例：

    ```bash
    curl http://localhost:20334/api/v1/node/state
    {
        "Desc": "Success",
        "Error": 0,
        "Result": {
            "Compile": "v0.2.0",
            "ID": 557625380943624900,
            "HexID": "0x7bd15545ce2fac4",
            "Height": 188850,
            "Version": 0,
            "Services": 0,
            "Relay": true,
            "TxnCnt": 84511,
            "RxTxnCnt": 21119,
            "Port": 20338,
            "PRCPort": 20336,
            "RestPort": 20334,
            "WSPort": 20335,
            "OpenPort": 20866,
            "OpenService": false,
            "Neighbors": [{
                "ID": 5225904035618466119,
                "HexID": "0x488624084951a947",
                "Height": 188850,
                "Services": 0,
                "Relay": true,
                "External": false,
                "State": "ESTABLISH",
                "NetAddress": "13.229.159.128:20338"
            }, {
                "ID": 14490220742601427650,
                "HexID": "0xc9179afe2fc14ac2",
                "Height": 188850,
                "Services": 4,
                "Relay": true,
                "External": false,
                "State": "ESTABLISH",
                "NetAddress": "52.220.199.55:20338"
            }]
        }
    }
    ```

* `/api/v1/block/height` : 获取节点区块高度

   示例：

    ```bash
    curl http://localhost:20334/api/v1/block/height
    {
        "Desc": "Success",
        "Error": 0,
        "Result": 1000
    }
    ```

* `/api/v1/transactionpool` : 获取节点交易池数据

    示例：

    ```bash
    curl http://localhost:20334/api/v1/transactionpool
    {
        "Desc": "Success",
        "Error": 0,
        "Result": [{
            "txid": "95d3bb263167581f2291de0ece4927ac8f5c2a9b10e448fa362e70cf6a11dfe5",
            "hash": "95d3bb263167581f2291de0ece4927ac8f5c2a9b10e448fa362e70cf6a11dfe5",
            "size": 368,
            "vsize": 368,
            "version": 0,
            "locktime": 173080,
            "vin": [{
                "txid": "1254a20e4a80274fdd477d13c7ebf8895b71ec6f2609c1f94f4e2601cfca019d",
                "vout": 0,
                "sequence": 0
            }],
            "vout": [{
                "value": "0.32300000",
                "n": 0,
                "address": "EeYC12CizLnApy9SDu9PW6jy2WCVZX5rHA",
                "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                "outputlock": 0
            }],
            "blockhash": "",
            "confirmations": 0,
            "time": 0,
            "blocktime": 0,
            "type": 5,
            "payloadversion": 0,
            "payload": {
                "BlockHeight": 20970,
                "SideBlockHash": "8ed6fbbb389b70b01e028e624c37a834e2f012daa88b179a3b4451f0721f6122",
                "SideGenesisHash": "a3c455a90843db2acd22554f2768a8d4233fafbf8dd549e6b261c2786993be56",
                "SignedData": "3c5d48777fbaf995e74cf386729495238af2b2b6043ecb15ff3fdb241f08f77884cb5bdab7bb43d7a4b25245e5e940861948613443cd92246667143471514cfb"
            },
            "attributes": [{
                "usage": 0,
                "data": "313530333634353830373333383735373432"
            }],
            "programs": [{
                "code": "2102776f2ad3fc822caa976bf0a83eb33cf4047518c9b6d411603be4a864b24acb4bac",
                "parameter": "406ef85b364b2d57a1369516ff334b16560068ae2947b9c9baa1a1f756802cc0d0ec659ed14dd73ec9af1ae1c6706304072ae40551ba4bbe8c2473f0ed6a8182e0"
            }]
        }]
    }
    ```

* `/api/v1/restart` : 重新启动节点restful服务

* `/api/v1/block/hash/<height>` : 根据区块 `height` 获取区块 `hash`

    示例：

    ```bash
    curl http://localhost:20334/api/v1/block/hash/123
    {
        "Desc": "Success",
        "Error": 0,
        "Result": "b9450e180aba1a96a93003be1ba775499577f856bb17a82177e74ae4d94db3fb"
    }
    ```

* `/api/v1/block/details/height/<height>` : 根据区块 `height` 获取区块详细信息

    示例：

    ```bash
    curl http://localhost:20334/api/v1/block/details/height/123
    {
        "Desc": "Success",
        "Error": 0,
        "Result": {
            "hash": "b9450e180aba1a96a93003be1ba775499577f856bb17a82177e74ae4d94db3fb",
            "confirmations": 188784,
            "strippedsize": 498,
            "size": 498,
            "weight": 1992,
            "height": 123,
            "version": 0,
            "versionhex": "00000000",
            "merkleroot": "3ce4c09723a6c241749098d7416bad64f2439920a8a8afdf19dac895cb946634",
            "tx": [{
                "txid": "3ce4c09723a6c241749098d7416bad64f2439920a8a8afdf19dac895cb946634",
                "hash": "3ce4c09723a6c241749098d7416bad64f2439920a8a8afdf19dac895cb946634",
                "size": 192,
                "vsize": 192,
                "version": 0,
                "locktime": 123,
                "vin": [{
                    "txid": "0000000000000000000000000000000000000000000000000000000000000000",
                    "vout": 65535,
                    "sequence": 4294967295
                }],
                "vout": [{
                    "value": "1.50684931",
                    "n": 0,
                    "address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
                    "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                    "outputlock": 0
                }, {
                    "value": "3.51598174",
                    "n": 1,
                    "address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
                    "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                    "outputlock": 0
                }],
                "blockhash": "b9450e180aba1a96a93003be1ba775499577f856bb17a82177e74ae4d94db3fb",
                "confirmations": 188784,
                "time": 1513940099,
                "blocktime": 1513940099,
                "type": 0,
                "payloadversion": 4,
                "payload": {
                    "CoinbaseData": "ELA"
                },
                "attributes": [{
                    "usage": 0,
                    "data": "2a92922b60b3e537"
                }],
                "programs": []
            }],
            "time": 1513940099,
            "mediantime": 1513940099,
            "nonce": 0,
            "bits": 520095999,
            "difficulty": "1",
            "chainwork": "0002e16f",
            "previousblockhash": "7f6effc4e37b0a29e1079d32f6e373e6a962d6531a4e9e5310433dd97709b2a4",
            "nextblockhash": "7cc40920292d89be189e63112f0d4f1ffc0b48cd9e17df0181035f7f0a27b1ba",
            "auxpow": "01000000010000000000000000000000000000000000000000000000000000000000000000000000002cfabe6d6dfbb34dd9e44ae77721a817bb56f877954975a71bbe0330a9961aba0a180e45b90100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffff7f0000000000000000000000000000000000000000000000000000000000000000a0573af605f4d829e3b397e5bd2a8832787eb0f48793152b9f5e8915fae5cc5683e43c5a0000000014a00000"
        }
    }
    ```

* `/api/v1/block/details/hash/<hash>` : 根据区块 `hash` 获取区块详细信息

    示例：

    ```bash
    curl http://localhost:20334/api/v1/block/details/hash/b9450e180aba1a96a93003be1ba775499577f856bb17a82177e74ae4d94db3fb
    {
        "Desc": "Success",
        "Error": 0,
        "Result": {
            "hash": "b9450e180aba1a96a93003be1ba775499577f856bb17a82177e74ae4d94db3fb",
            "confirmations": 188788,
            "strippedsize": 498,
            "size": 498,
            "weight": 1992,
            "height": 123,
            "version": 0,
            "versionhex": "00000000",
            "merkleroot": "3ce4c09723a6c241749098d7416bad64f2439920a8a8afdf19dac895cb946634",
            "tx": ["3ce4c09723a6c241749098d7416bad64f2439920a8a8afdf19dac895cb946634"],
            "time": 1513940099,
            "mediantime": 1513940099,
            "nonce": 0,
            "bits": 520095999,
            "difficulty": "1",
            "chainwork": "0002e173",
            "previousblockhash": "7f6effc4e37b0a29e1079d32f6e373e6a962d6531a4e9e5310433dd97709b2a4",
            "nextblockhash": "7cc40920292d89be189e63112f0d4f1ffc0b48cd9e17df0181035f7f0a27b1ba",
            "auxpow": "01000000010000000000000000000000000000000000000000000000000000000000000000000000002cfabe6d6dfbb34dd9e44ae77721a817bb56f877954975a71bbe0330a9961aba0a180e45b90100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffff7f0000000000000000000000000000000000000000000000000000000000000000a0573af605f4d829e3b397e5bd2a8832787eb0f48793152b9f5e8915fae5cc5683e43c5a0000000014a00000"
        }
    }
    ```
    
    
* `/api/v1/confirm/details/height/<height>` : 根据区块 `height` 获取区块确认详细信息

    Example:

    ```bash
    curl http://localhost:20334/api/v1/confirm/details/height/123
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

* `/api/v1/confirm/details/hash/<hash>` : 根据区块 `hash` 获取区块确认详细信息

    Example:

    ```bash
    curl http://localhost:20334/api/v1/confirm/details/hash/65fd07f4069a1ca6833d38e0baa9b0b4a9bac35131feb5c363bd2fb99f8d06d8
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

* `/api/v1/block/transactions/height/<height>` : 根据区块 `height` 获取区块所有交易 `hash`

   示例：

    ```bash
    curl http://localhost:20334/api/v1/block/transactions/height/16000
    {
        "Desc": "Success",
        "Error": 0,
        "Result": {
            "Hash": "3fe8b94a27cccbbcd4a6675e1f3df62f0571fc847abf6488d7fff6e522d96862",
            "Height": 16000,
            "Transactions": ["5c0a30ad3764e6524803ac673d552af59acac910cea36f1678d5b0ae03c67078", "73d201de3cc1ccce92d6edff03cf4d1741c6ab99118dce37f7027eec4a5b3e93"]
        }
    }
    ```

* `/api/v1/transaction/<hash>` : 根据交易 `hash` 获取区块所有交易信息

    示例：

    ```bash
    curl http://localhost:20334/api/v1/transaction/5c0a30ad3764e6524803ac673d552af59acac910cea36f1678d5b0ae03c67078
    {
        "Desc": "Success",
        "Error": 0,
        "Result": {
            "txid": "5c0a30ad3764e6524803ac673d552af59acac910cea36f1678d5b0ae03c67078",
            "hash": "5c0a30ad3764e6524803ac673d552af59acac910cea36f1678d5b0ae03c67078",
            "size": 192,
            "vsize": 192,
            "version": 0,
            "locktime": 16000,
            "vin": [{
                "txid": "0000000000000000000000000000000000000000000000000000000000000000",
                "vout": 65535,
                "sequence": 4294967295
            }],
            "vout": [{
                "value": "1.50686389",
                "n": 0,
                "address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
                "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                "outputlock": 0
            }, {
                "value": "3.51601576",
                "n": 1,
                "address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
                "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                "outputlock": 0
            }],
            "blockhash": "3fe8b94a27cccbbcd4a6675e1f3df62f0571fc847abf6488d7fff6e522d96862",
            "confirmations": 172872,
            "time": 1515940214,
            "blocktime": 1515940214,
            "type": 0,
            "payloadversion": 4,
            "payload": {
                "CoinbaseData": "ELA"
            },
            "attributes": [{
                "usage": 0,
                "data": "c9a7a12d61c446d4"
            }],
            "programs": []
        }
    }
    ```

* `/api/v1/asset/balances/<addr>` : 根据钱包地址获取钱包余额

    示例：

    ```bash
    curl http://localhost:20334/api/v1/asset/balances/EgHPRhodCsDKuDBPApCK3KLayiBomrJrbH
    {
        "Desc": "Success",
        "Error": 0,
        "Result": "20.84298920"
    }
    ```

* `/api/v1/asset/<hash>` : 资产查询

    示例：

    ```bash
    curl http://localhost:20334/api/v1/asset/a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0
    {
        "Desc": "Success",
        "Error": 0,
        "Result": {
            "Name": "ELA",
            "Description": "",
            "Precision": 8,
            "AssetType": 0,
            "RecordType": 0
        }
    }
    ```

* `/api/v1/asset/utxos/<addr>` : 获取一个地址所有 `UTXO`

    示例：

    ```bash
    curl http://localhost:20334/api/v1/asset/utxos/EgHPRhodCsDKuDBPApCK3KLayiBomrJrbH
    {
        "Desc": "Success",
        "Error": 0,
        "Result": [{
            "AssetId": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
            "AssetName": "ELA",
            "Utxo": [{
                "Txid": "c8d4dc984da78c878b9dab752c077b41a98f6e67e5ee6b04cc3d45cb4f42b81b",
                "Index": 1,
                "Value": "0.09956920"
            }, {
                "Txid": "0b219b2b5b836dfa6acb10fad653fadd384494df3f6710ce168c6055106d101b",
                "Index": 0,
                "Value": "20.74342000"
            }]
        }]
    }
    ```

* `/api/v1/asset/balance/<addr>/<assetid>` : 根据地址和AssetID查询余额

    示例：

    ```bash
    curl http://localhost:20334/api/v1/asset/balance/EgHPRhodCsDKuDBPApCK3KLayiBomrJrbH/a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0
    {
        "Desc": "Success",
        "Error": 0,
        "Result": "20.84298920"
    }
    ```

* `/api/v1/asset/utxo/<addr>/<assetid>` : 根据地址和AssetID查询UTXO

    示例：

    ```bash
    curl http://localhost:20334/api/v1/asset/utxo/EgHPRhodCsDKuDBPApCK3KLayiBomrJrbH/a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0
    {
        "Desc": "Success",
        "Error": 0,
        "Result": [{
            "Txid": "c8d4dc984da78c878b9dab752c077b41a98f6e67e5ee6b04cc3d45cb4f42b81b",
            "Index": 1,
            "Value": "0.09956920"
        }, {
            "Txid": "0b219b2b5b836dfa6acb10fad653fadd384494df3f6710ce168c6055106d101b",
            "Index": 0,
            "Value": "20.74342000"
        }]
    }
    ```

* `/api/v1/transaction` : 将签名后的交易数据广播至节点

    示例：

    ```bash
    curl -X POST http://localhost:21334/api/v1/transaction -H "Content-Type: application/json" -d '{"Action": "sendrawtransaction", "data": "020001001335353737303036373931393437373739343130010faf7e6f2f43ebdc2723e50014280d4cdac1975f9f883dc57e60aa7e96047b5401000000000002b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a300e1f505000000000000000021132c86ebad33299ecc15dff410d4b0a76b4f9e17b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3a02bb459e4050000000000002136c6f2ad6785cef94fab1216b776add14bd756a850ab0100014140cbd0fb92a390dcd802ecef745091045ccb3033f097e6a42cc18822b000360e48d1a09388db077ca9c7cfc889c0eca427ca1f5f05490658a854f752ea1fb57b7c2321032f4540e915134f38ba24cdc08621ad7f5b8b62db36843ae8fa9422c047a04be8ac"}'
    {
        "Desc": "Success",
        "Error": 0,
        "Result": "2e8d51bdbba82af7a7ed334cb0fb60ad9a5da7e5170f9d2509023f3ed3cce1d0"
    }
    ```

    这个接口里面data参数值(签名后的交易数据)的生成可以使用两种方法：


    1. 使用 [ela-cli](https://github.com/elastos/Elastos.ELA/blob/master/docs/cli_user_guide_CN.md) 命令行工具，示例如下
    ```bash
    ./ela-cli wallet buildtx --from EdAEC51BmmzJFHUdMJ6bR5fZB4oo919E8n --to EgKxcDE1kfEXbpXzwL8eavk6XxMyGEqXmC --amount 10 --fee 0.01
    ```

    请使用实际的ELA Address替换示例中的 --from 和 --to 的参数值
    ```bash
    ./ela-cli wallet signtx -f to_be_signed.txn
    ```

    这里需要输入你本地钱包的密码，这个命令返回的一长串十六进制的字符就是签名后的交易数据

    2. 使用 [Elastos.ELA.Utilities.Java](https://github.com/elastos/Elastos.ELA.Utilities.Java) 工具库提供的相关工具生成，具体参考仓库的文档
