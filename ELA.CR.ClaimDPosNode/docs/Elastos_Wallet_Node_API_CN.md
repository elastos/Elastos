# The Node API Of Elastos Wallet

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

* `/api/v1/block/height` : 获取节点区块总高度

* `/api/v1/transactionpool` : 获取节点交易池数据

* `/api/v1/restart` : 重新启动节点服务器

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

* `/api/v1/block/details/hash/<hash>` : 根据区块 `hash` 获取区块详细信息

* `/api/v1/block/transactions/height/<height>` : 根据区块 `height` 获取区块所有交易 `hash`

* `/api/v1/transaction/<hash>` : 根据交易 `hash` 获取区块所有交易信息

* `/api/v1/asset/balances/<addr>` : 根据钱包地址获取钱包余额

    示例：

    ```bash
    curl http://localhost:20334/api/v1/asset/balances/ES4p9GBXV4n8PayEPyiEmCrjKoRXTfYR4Q
    {
    	"Desc": "Success",
    	"Error": 0,
    	"Result": "390.49022408"
    }
    ```

* `/api/v1/asset/<hash>` : 资产查询

* `/api/v1/asset/utxos/<addr>` : 获取一个地址所有交易 `UTXO`

* `/api/v1/asset/balance/<addr>/<assetid>` : 根据地址和AssetID查询余额

* `/api/v1/asset/utxo/<addr>/<assetid>` : 根据地址和AssetID查询UTXO
