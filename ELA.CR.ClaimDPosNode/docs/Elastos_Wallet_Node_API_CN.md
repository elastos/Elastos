# The Node API Of Elastos Wallet

`ELA Node` 使用 `2*334` 端口提供如下接口服务：

* `/api/v1/node/connectioncount` : 获取当前节点所连接到的节点数量

   示例：

    ```bash
    curl http://localhost:20334/api/v1/node/connectioncount

    {
        "Action": "getconnectioncount",
        "Desc": "SUCCESS",
        "Error": 0,
        "Result": 8,
        "Version": "1.0.0"
    }
    ```

* `/api/v1/block/height` : 获取节点区块总高度

* `/api/v1/transactionpool` : 获取节点交易池数据

* `/api/v1/restart` : 重新启动节点服务器

* `/api/v1/block/hash/<height>` : 根据区块 `height` 获取区块 `hash`

    示例：

    ```bash
    curl http://localhost:20334/api/v1/block/hash/123

    {
        "Action": "getblockhash",
        "Desc": "SUCCESS",
        "Error": 0,
        "Result": "a8678481a8f5bed4d277d0bbac98b52d7ce3aa6f76043149cb4021c1a40d201a",
        "Version": "1.0.0"
    }
    ```

* `/api/v1/block/details/height/<height>` : 根据区块 `height` 获取区块详细信息

* `/api/v1/block/details/hash/<hash>` : 根据区块 `hash` 获取区块详细信息

* `/api/v1/block/transactions/height/<height>` : 根据区块 `height` 获取区块所有交易信息

* `/api/v1/transaction/<hash>` : 根据交易 `hash` 获取区块所有交易信息

* `/api/v1/asset/balances/<addr>` : 根据钱包地址获取钱包余额

    示例：

    ```bash
    curl http://localhost:20334/api/v1/asset/balances/ES4p9GBXV4n8PayEPyiEmCrjKoRXTfYR4Q

    {
    "Action":"getbalancebyaddr",
    "Desc": "Success",
    "Error": 0,
    "Result": "7696.48402886",
    "Version":"1.0.0"
    }
    ```

* `/api/v1/asset/<hash>` : 资产查询

* `/api/v1/asset/utxos/<addr>` : 获取一个地址所有交易 `UTXO`

* `/api/v1/asset/balance/<addr>/<assetid>` : 根据地址和AssetID查询余额

* `/api/v1/asset/utxo/<addr>/<assetid>` : 根据地址和AssetID查询UTXO

* `/api/v1/transaction` :     创建一笔交易

    示例：

    ```bash
    #交易需要两步处理

    #首先使用如下请求方法获取 RawTransaction
    {
        "method": "genRawTransaction",
        "id": 0,
        "params": [{
            "Transactions": [{
                "UTXOInputs": [{
                    "txid": "61c22a83bb96d958f473148fa64f3b2be02653c66ede506e83b82e522200d446",
                    "index": 0,
                    "privateKey": "5FA927E5664E563F019F50DCD4D7E2D9404F2D5D49E31F9482912E23D6D7B9EB",
                    "address": "EQSpUzE4XYJhBSx5j7Tf2cteaKdFdixfVB"
                }, {
                    "txid": "a91b63ba6ffdb13379451895c51abd25c54678bc89268db6e6c3dcbb7bb07062",
                    "index": 0,
                    "privateKey": "A65E9FB6735C5FD33F839036B15D2DA373E15AED38054B69386E322C6BE52994",
                    "address": "EgSph8GNaNSMwpv6UseAihsAc5sqSrA7ga"
                }],
                "Outputs": [{
                    "address": "ERz34iKa4nGaGYVtVpRWQZnbavJEe6PRDt",
                    "amount": 200
                }, {
                    "address": "EKjeZEmLSXyyJ42xxjJP4QsKJYWwEXabuC",
                    "amount": 240
                }]
            }]
        }]
    }

    #然后使用返回的 rawTx 值 作为 RawTransaction 构建如下请求

    {
        "method": "decodeRawTransaction",
        "id": 0,
        "params": [{
            "RawTransaction": "02000100142D37323733373430363730363936353637333337015220F787A81709244D9987606E77A74411F61D7E20930924F81A1F4815DEBA2200000000000001B037DB964A231458D2D6FFD5EA18944C4F90E63D547C5D3B9874DF66A4EAD0A30070AE1993A70A000000000021C3B5C32D6FE7CAC86A855276D087C443FB12178B00000000014140E62D5E3E8E14B33377F7EA7301968B81163959A572178CC555F184B2F5239BB683B62E6F178E4C07D6B0D43F780A289488634E4B477197196B8F95581ACA1322232102EE009B86F9377820B1DE396888E7456FDE2554E77E1D9A1AB3360562F1D6FF4BAC"
        }]
    }
    ```