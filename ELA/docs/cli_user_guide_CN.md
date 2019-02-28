# ela-cli 使用说明

## 1.钱包管理

```
NAME:
   ela-cli wallet - With ela-cli wallet, you could control your asset.

USAGE:
   ela-cli wallet command [command options] [args]

COMMANDS:
   Account:
     create, c    Create a account
     account, a   Show account address and public key
     balance, b   Check account balance
     add          Add a standard account
     addmultisig  Add a multi-signature account
     delete       Delete an account
     import       Import an account by private key hex string
     export       Export all account private keys in hex string
     depositaddr  Generate deposit address

   Transaction:
     buildtx  Build a transaction
     signtx   Sign a transaction
     sendtx   Send a transaction
     showtx   Show info of raw transaction

OPTIONS:
   --help, -h  show help
```

### 1.1 创建账户

创建账户命令用于创建一个单签账户，并将私钥加密存储在 keystore 文件中。每个 keystore 文件都有一个**主账户**，一般情况下是第一个被添加上账户。

--wallet <file>, -w <file> 用于指定 keystore 文件路径。若不指定，默认为 `keystore.dat` 文件。

--password <value>, -p <value> 用于指定 keystore 密码。也可以在下一步提示时再输入密码。

```
./ela-cli wallet create -p 123
```

返回如下：

```
ADDRESS                            PUBLIC KEY
---------------------------------- ------------------------------------------------------------------
ESVMKLVB1j1KQR8TYP7YbksHpNSHg5NZ8i 032d3d0e8125ac6215c237605486c9fbd4eb764f52f89faef6b7946506139d26f8
---------------------------------- ------------------------------------------------------------------
```

### 1.2 查询公钥

查询 keystore 文件中所有账户地址及其公钥。可以使用 -w 指定 keystore 文件，默认查询 `keystore.dat`。

```
./ela-cli wallet account -p 123
```

返回如下：

```
ADDRESS                            PUBLIC KEY
---------------------------------- ------------------------------------------------------------------
ESVMKLVB1j1KQR8TYP7YbksHpNSHg5NZ8i 032d3d0e8125ac6215c237605486c9fbd4eb764f52f89faef6b7946506139d26f8
---------------------------------- ------------------------------------------------------------------
```

### 1.3 查询余额

查询 keystore 文件中所有账户余额。可以使用 -w 指定 keystore 文件，默认查询 `keystore.dat`。

```
./ela-cli wallet balance
```

返回如下：

```
INDEX                            ADDRESS BALANCE                           (LOCKED)
----- ---------------------------------- ------------------------------------------
    0 EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR 505.08132198                (174.04459514)
----- ---------------------------------- ------------------------------------------
    0 8PT1XBZboe17rq71Xq1CvMEs8HdKmMztcP 10                                     (0)
----- ---------------------------------- ------------------------------------------
```

### 1.4 添加单签账户

可以使用 -w 指定 keystore 文件，默认查询 `keystore.dat`。

```
./ela-cli wallet add
```

返回如下：

```
ADDRESS                            PUBLIC KEY
---------------------------------- ------------------------------------------------------------------
ET15giWpFNSYcTKVbj3s18TsR6i8MBnkvk 031c862055158e50dd6e2cf6bb33f869aaac42c5e6def66a8c12efc1fdd16d5597
---------------------------------- ------------------------------------------------------------------
```

### 1.5 添加多签地址

添加多签地址需要指定公钥列表pks，以及在公钥列表中所需要的最少签名数量m。可以使用 -w 指定 keystore 文件，默认查询 `keystore.dat`。

-- pks <pubkey list> 用于设定公钥列表，公钥之间以 `,` 分开。

-- m <value>, -m <value> 用于设定最少签名数量。无符号整型。

生成一个 3/4 签名的多签地址：

```
./ela-cli wallet addmultisig -m 3 --pks 0325406f4abc3d41db929f26cf1a419393ed1fe5549ff18f6c579ff0c3cbb714c8,0353059bf157d3eaca184cc10a80f10baf10676c4a39695a9a092fa3c2934818bd,03fd77d569f766638677755e8a71c7c085c51b675fbf7459ca1094b29f62f0b27d,0353059bf157d3eaca184cc10a80f10baf10676c4a39695a9a092fa3c2934818bd
```

根据提示，输入 keystore 文件密码。

返回如下：

```
8PT1XBZboe17rq71Xq1CvMEs8HdKmMztcP
```

此时`8PT1XBZboe17rq71Xq1CvMEs8HdKmMztcP` 被加入到指定 keystore 文件中。

### 1.6 删除账户

删除账户命令用于删除 keystore 中某个账户。第一个被创建的账户，即主账户不可被删除。

查看 keystore 中账户情况：

```
./ela-cli wallet account
```

返回如下：

```
ADDRESS                            PUBLIC KEY
---------------------------------- ------------------------------------------------------------------
ET15giWpFNSYcTKVbj3s18TsR6i8MBnkvk 031c862055158e50dd6e2cf6bb33f869aaac42c5e6def66a8c12efc1fdd16d5597
---------------------------------- ------------------------------------------------------------------
EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR 034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16c
---------------------------------- ------------------------------------------------------------------
```

删除账户：

```
./ela-cli wallet delete ET15giWpFNSYcTKVbj3s18TsR6i8MBnkvk
```

返回如下：

```
ADDRESS                            PUBLIC KEY
---------------------------------- ------------------------------------------------------------------
EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR 034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16c
---------------------------------- ------------------------------------------------------------------
```

### 1.7 导出账户

导出账户命令用于导出某个 keystore 文件中所有账户的私钥。可用 -w 指定 keystore 文件。

```
./ela-cli wallet export
```

根据提示输入密码，返回如下：

```
ADDRESS                            PRIVATE KEY
---------------------------------- ------------------------------------------------------------------
EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR c779d181658b112b584ce21c9ea3c23d2be0689550d790506f14bdebe6b3fe38
---------------------------------- ------------------------------------------------------------------
```

### 1.8 导入账户

导入账户命令用于导入私钥至 keystore 文件中。可用 -w 指定 keystore 文件。

```
./ela-cli wallet import b2fe3300e44b27b199d97af43ed3e82df4670db66727732ba8d9442ce680da35 -w keystore1.dat
```

根据提示输入密码，返回如下：

```
ADDRESS                            PUBLIC KEY
---------------------------------- ------------------------------------------------------------------
EQJP3XT7rshteqE1D3u9nBqXL7xQrfzVh1 03f69479d0f6aa11aae5fbe5e5bfca201d717d9fa97a45ca7b89544047a1a30f59
---------------------------------- ------------------------------------------------------------------
```

### 1.9 生成押金地址

生成押金地址命令可由指定地址生成对应的押金地址。

```
./ela-cli wallet depositaddr EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR
```

返回如下：

```
DVgnDnVfPVuPa2y2E4JitaWjWgRGJDuyrD
```

### 1.10 构造交易

构造交易命令 buildtx 用于构造转账交易的内容，构造出来的交易在发送到 ela 节点前，还需要用的私钥签名。

-- from <address> 参数用于设定花费地址。如果不设定，默认使用 keystore.dat 中主地址。

-- to <address> 参数用于设定收款地址。

-- amount <amount> 参数用于设定转账金额。浮点类型。

-- fee <fee> 参数用于设定交易的手续费。浮点类型。

-- wallet <file>, -w <file> 参数用于设定所使用的 keystore 文件路径。

#### 1.10.1 构造单签交易

```
./ela-cli wallet buildtx --to EJbTbWd8a9rdutUfvBxhcrvEeNy21tW1Ee --amount 0.1 --fee 0.01
```

返回如下：

```
Hex:  090200010013373934373338313938363837333037373435330139eaa4137a047cdaac7112fe04617902e3b17c1685029d259d93c77680c950f30100ffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3809698000000000000000000210fcd528848be05f8cffe5d99896c44bdeec7050200b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a36ea2d2090000000000000000210d4109bf00e6d782db40ab183491c03cf4d6a37a000000000001002321034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16cac
File:  to_be_signed.txn
```

#### 1.10.2 构造多签交易

from 地址需要存在于所指定的 keystore 文件中，可使用 -w 参数指定 keystore 文件，默认为 keystore.dat。

```
./ela-cli wallet buildtx --from 8PT1XBZboe17rq71Xq1CvMEs8HdKmMztcP --to EQJP3XT7rshteqE1D3u9nBqXL7xQrfzVh1 --amount 0.51 --fee 0.001
```

返回如下：

```
Hex:  0902000100133334393234313234323933333338333335313701737a31035ebe8dfe3c58c7b9ff7eb13485387cd2010d894f39bf670ccd1f62180000ffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3c0320a030000000000000000214e6334d41d86e3c3a32698bdefe974d6960346b300b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3a0108f380000000000000000125bc115b91913c9c6347f0e0e3ba3b75c80b94811000000000001008b53210325406f4abc3d41db929f26cf1a419393ed1fe5549ff18f6c579ff0c3cbb714c8210353059bf157d3eaca184cc10a80f10baf10676c4a39695a9a092fa3c2934818bd210353059bf157d3eaca184cc10a80f10baf10676c4a39695a9a092fa3c2934818bd2103fd77d569f766638677755e8a71c7c085c51b675fbf7459ca1094b29f62f0b27d54ae
File:  to_be_signed.txn
```

### 1.11 对交易签名

使用 buildtx 命令构造的交易，需要通过花费地址的私钥签名后，才是有效的交易。

-- hex <value> 用于设定待签名交易的 hex 字符串。

-- file <file> 用于设定待签名交易 hex 字符串所在的文件路径。-- file 与 -- hex 选一种方式即可。

-- wallet <file>, -w <file> 用于设定私钥所在 keystore 文件路径。如果不设定，默认使用 keystore.dat 的主账户。

-- password <value>, -p <value> 用于设定 keystore 密码。也可以根据提示设定。

#### 1.11.1 单签

指定构造好的单签交易：

```
./ela-cli wallet signtx -f to_be_signed.txn
```

根据提示，输入 keystore 密码。

返回如下：

```
[ 1 / 1 ] Transaction successfully signed
Hex:  090200010013373934373338313938363837333037373435330139eaa4137a047cdaac7112fe04617902e3b17c1685029d259d93c77680c950f30100ffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3809698000000000000000000210fcd528848be05f8cffe5d99896c44bdeec7050200b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a36ea2d2090000000000000000210d4109bf00e6d782db40ab183491c03cf4d6a37a0000000000014140b3963ac174e922592905154c34519765c246441132707ccd6b4fb98d6107cc391e180d106bb02ce8e895d4662eb353f72f4859366ddf01fb97ae32f39bf6d1752321034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16cac
File:  ready_to_send.txn
```

#### 1.11.2 多签

指定构造好的多签交易，附加第一个签名：

```
./ela-cli wallet signtx -f to_be_signed.txn -w keystore1.dat
```

返回如下：

```
[ 1 / 3 ] Transaction successfully signed
Hex:  0902000100133334393234313234323933333338333335313701737a31035ebe8dfe3c58c7b9ff7eb13485387cd2010d894f39bf670ccd1f62180000ffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3c0320a030000000000000000214e6334d41d86e3c3a32698bdefe974d6960346b300b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3a0108f380000000000000000125bc115b91913c9c6347f0e0e3ba3b75c80b9481100000000000141406952da35f525b4db087654f2b0317b6ea7d385d22539f5ecf2001cc6db169be0b6945e073180796f2a070a29e90c1c62e5b9a3f8b3f040b3fe8ea6be57da5a858b53210325406f4abc3d41db929f26cf1a419393ed1fe5549ff18f6c579ff0c3cbb714c8210353059bf157d3eaca184cc10a80f10baf10676c4a39695a9a092fa3c2934818bd210353059bf157d3eaca184cc10a80f10baf10676c4a39695a9a092fa3c2934818bd2103fd77d569f766638677755e8a71c7c085c51b675fbf7459ca1094b29f62f0b27d54ae
File:  to_be_signed_1_of_3.txn
```

附加第二个签名：

```
./ela-cli wallet signtx -f to_be_signed_1_of_3.txn -w keystore2.dat
```

返回如下：

```
[ 2 / 3 ] Transaction successfully signed
Hex:  0902000100133334393234313234323933333338333335313701737a31035ebe8dfe3c58c7b9ff7eb13485387cd2010d894f39bf670ccd1f62180000ffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3c0320a030000000000000000214e6334d41d86e3c3a32698bdefe974d6960346b300b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3a0108f380000000000000000125bc115b91913c9c6347f0e0e3ba3b75c80b9481100000000000182406952da35f525b4db087654f2b0317b6ea7d385d22539f5ecf2001cc6db169be0b6945e073180796f2a070a29e90c1c62e5b9a3f8b3f040b3fe8ea6be57da5a854089aef5f476793d22f2403b575463129041bc37392109f3a96b43a4a94876b43991d40b8fd0ee591a6faa948aea9053fa7a767a21d8772958c498dfb753f576d68b53210325406f4abc3d41db929f26cf1a419393ed1fe5549ff18f6c579ff0c3cbb714c8210353059bf157d3eaca184cc10a80f10baf10676c4a39695a9a092fa3c2934818bd210353059bf157d3eaca184cc10a80f10baf10676c4a39695a9a092fa3c2934818bd2103fd77d569f766638677755e8a71c7c085c51b675fbf7459ca1094b29f62f0b27d54ae
File:  to_be_signed_2_of_3.txn
```

附加第三个签名：

```
./ela-cli wallet signtx -f to_be_signed_2_of_3.txn -w keystore3.dat
```

返回如下：

```
[ 3 / 3 ] Transaction successfully signed
Hex:  0902000100133334393234313234323933333338333335313701737a31035ebe8dfe3c58c7b9ff7eb13485387cd2010d894f39bf670ccd1f62180000ffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3c0320a030000000000000000214e6334d41d86e3c3a32698bdefe974d6960346b300b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3a0108f380000000000000000125bc115b91913c9c6347f0e0e3ba3b75c80b94811000000000001c3406952da35f525b4db087654f2b0317b6ea7d385d22539f5ecf2001cc6db169be0b6945e073180796f2a070a29e90c1c62e5b9a3f8b3f040b3fe8ea6be57da5a854089aef5f476793d22f2403b575463129041bc37392109f3a96b43a4a94876b43991d40b8fd0ee591a6faa948aea9053fa7a767a21d8772958c498dfb753f576d6408dfd58568cd95be995fa69ab9924c28c23fdc738caf19fb243a285c1cda01793d676c0459d546e92cd69b4f9837f750adc3b6f66fe0eff5dcd5a9cc46c1ec6138b53210325406f4abc3d41db929f26cf1a419393ed1fe5549ff18f6c579ff0c3cbb714c8210353059bf157d3eaca184cc10a80f10baf10676c4a39695a9a092fa3c2934818bd210353059bf157d3eaca184cc10a80f10baf10676c4a39695a9a092fa3c2934818bd2103fd77d569f766638677755e8a71c7c085c51b675fbf7459ca1094b29f62f0b27d54ae
File:  ready_to_send.txn
```

### 1.12 发送交易

发送交易命令可以将已签名完成的交易发送至 ela 节点。

```
./ela-cli wallet sendtx -f ready_to_send.txn
```

返回如下：

```
5b9673a813b90dd73f6d21f478736c7e08bba114c3772618fca232341af683b5
```

### 1.13 查看交易

查看交易命令可以解析原始交易字节中的内容。可使用 --hex 或 --file 指定原始交易。

```
./ela-cli wallet showtx --hex 0902000100123132333835343835313135373533343433340139eaa4137a047cdaac7112fe04617902e3b17c1685029d259d93c77680c950f30100ffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a380778e06000000000000000012e194f97570ada85ec5df31e7c192edf1e3fc199900b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a36ec1dc030000000000000000210d4109bf00e6d782db40ab183491c03cf4d6a37a0000000000014140433f2f2fa7390db75af3a3288943ce178f9373b2362a3b09530dd30fbb50fa5f007716bf07abd7ee0281d422f3615b474ed332ba63fe7209f611e547d68e4c0f2321034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16cac
```

返回如下：

```
Transaction: {
	Hash: 4661854b9441d52f78f5f4c025b37270af3aaa60332c4bfe041fa3cbc91b0a7e
	Version: 9
	TxType: TransferAsset
	PayloadVersion: 0
	Payload: 00
	Attributes: [Attribute: {
		Usage: Nonce
		Data: 313233383534383531313537353334343334
		}]
	Inputs: [{TxID: 39eaa4137a047cdaac7112fe04617902e3b17c1685029d259d93c77680c950f3 Index: 1 Sequence: 4294967295}]
	Outputs: [Output: {
	AssetID: b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3
	Value: 1.10000000
	OutputLock: 0
	ProgramHash: 12e194f97570ada85ec5df31e7c192edf1e3fc1999
	Type: 0
	Payload: &{}
	} Output: {
	AssetID: b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3
	Value: 0.64799086
	OutputLock: 0
	ProgramHash: 210d4109bf00e6d782db40ab183491c03cf4d6a37a
	Type: 0
	Payload: &{}
	}]
	LockTime: 0
	Programs: [Program: {
		Code: 21034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16cac
		Parameter: 40433f2f2fa7390db75af3a3288943ce178f9373b2362a3b09530dd30fbb50fa5f007716bf07abd7ee0281d422f3615b474ed332ba63fe7209f611e547d68e4c0f
		}]
	}
```

## 2.信息查询

```
NAME:
   ela-cli info - With ela-cli info, you could look up node status, query blocks, transactions, etc.

USAGE:
   ela-cli info command [command options] [args]

COMMANDS:
     getconnectioncount  Show how many peers are connected
     getneighbors        Show neighbor nodes information
     getnodestate        Show current node status
     getcurrentheight    Get best block height
     getbestblockhash    Get the best block hash
     getblockhash        Get a block hash by height
     getblock            Get a block details by height or block hash
     getrawtransaction   Get raw transaction by transaction hash
     getrawmempool       Get transaction details in node mempool

OPTIONS:
   --help, -h  show help
```

### 2.1 查看节点连接数

```
./ela-cli info getconnectioncount
```

返回连接数：

```
1
```

### 2.2 查看邻居节点

```
./ela-cli info getneighbors
```

返回邻居节点信息：

```
[
    "127.0.0.1:30338 (outbound)"
]
```

### 2.3 查看节点状态

```
./ela-cli info getnodestate
```

返回节点状态列表：

```
[
    {
        "Addr": "127.0.0.1:30338",
        "ConnTime": "2019-02-28T14:32:44.996711+08:00",
        "ID": 4130841897781718000,
        "Inbound": false,
        "LastBlock": 395,
        "LastPingMicros": 0,
        "LastPingTime": "0001-01-01T00:00:00Z",
        "LastRecv": "2019-02-28T14:32:45+08:00",
        "LastSend": "2019-02-28T14:32:45+08:00",
        "RelayTx": 0,
        "Services": 3,
        "StartingHeight": 395,
        "TimeOffset": 0,
        "Version": 10002
    }
]
```

### 2.4 获取节点当前高度

```
./ela-cli info getcurrentheight
```

返回节点当前高度：

```
395
```

### 2.5 获取最高区块hash

```
./ela-cli info getbestblockhash
```

返回最高块 hash：

```
"0affad77eacef8d5e69bebd1edd24b43ca8d8948dade9e23b14a9d8ceca060e6"
```

### 2.6 获取区块 hash

获取块高为 100 的区块 hash

```
./ela-cli info getblockhash 100
```

返回区块 hash：

```
"1c1e1c22ce891184d390def30a9b8f15f355c05a7bd6e7e7912b571141e01415"
```

### 2.7 获取区块信息

通过区块 hash 获取区块信息：

```
./ela-cli info getblock 1c1e1c22ce891184d390def30a9b8f15f355c05a7bd6e7e7912b571141e01415
```

返回如下：

```
{
    "auxpow": "01000000010000000000000000000000000000000000000000000000000000000000000000000000002cfabe6d6d1514e04111572b91e7e7d67b5ac055f3158f9b0af3de90d3841189ce221c1e1c0100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffff7f000000000000000000000000000000000000000000000000000000000000000073f93ed7b4401e54aef152505f4d4b88c7c5462453e77069455262bed04575df3f236e5c00000000e6ef0600",
    "bits": 520095999,
    "chainwork": "00000127",
    "confirmations": 296,
    "difficulty": "1",
    "hash": "1c1e1c22ce891184d390def30a9b8f15f355c05a7bd6e7e7912b571141e01415",
    "height": 100,
    "mediantime": 1550721855,
    "merkleroot": "8570ff4cd3d356be2f9d90333205f9d4b11bd234aa513bf16c53c503f6575718",
    "minerinfo": "",
    "nextblockhash": "0c1a3bddf4686c6be9be8c4be2d3915c542856adac8c7458bd056df83e434eaa",
    "nonce": 0,
    "previousblockhash": "05d6db028304da9c126ea34a2820bfda8a269e2ceb9bfdda73e70632ef13be7f",
    "size": 560,
    "strippedsize": 560,
    "time": 1550721855,
    "tx": [
        {
            "attributes": [
                {
                    "data": "33c33ebc08ea3c1a",
                    "usage": 0
                }
            ],
            "blockhash": "1c1e1c22ce891184d390def30a9b8f15f355c05a7bd6e7e7912b571141e01415",
            "blocktime": 1550721855,
            "confirmations": 296,
            "hash": "8570ff4cd3d356be2f9d90333205f9d4b11bd234aa513bf16c53c503f6575718",
            "locktime": 100,
            "payload": {
                "CoinbaseData": ""
            },
            "payloadversion": 4,
            "programs": [],
            "size": 254,
            "time": 1550721855,
            "txid": "8570ff4cd3d356be2f9d90333205f9d4b11bd234aa513bf16c53c503f6575718",
            "type": 0,
            "version": 0,
            "vin": [
                {
                    "sequence": 4294967295,
                    "txid": "0000000000000000000000000000000000000000000000000000000000000000",
                    "vout": 65535
                }
            ],
            "vout": [
                {
                    "address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
                    "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                    "n": 0,
                    "outputlock": 0,
                    "payload": null,
                    "type": 0,
                    "value": "1.50684931"
                },
                {
                    "address": "EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR",
                    "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                    "n": 1,
                    "outputlock": 0,
                    "payload": null,
                    "type": 0,
                    "value": "1.75799086"
                },
                {
                    "address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
                    "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                    "n": 2,
                    "outputlock": 0,
                    "payload": null,
                    "type": 0,
                    "value": "1.75799088"
                }
            ],
            "vsize": 254
        }
    ],
    "version": 0,
    "versionhex": "00000000",
    "weight": 2240
}
```

通过区块高度过去区块信息：

```
./ela-cli info getblock 100
```

返回如下：

```
{
    "auxpow": "01000000010000000000000000000000000000000000000000000000000000000000000000000000002cfabe6d6d1514e04111572b91e7e7d67b5ac055f3158f9b0af3de90d3841189ce221c1e1c0100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000ffffff7f000000000000000000000000000000000000000000000000000000000000000073f93ed7b4401e54aef152505f4d4b88c7c5462453e77069455262bed04575df3f236e5c00000000e6ef0600",
    "bits": 520095999,
    "chainwork": "00000127",
    "confirmations": 296,
    "difficulty": "1",
    "hash": "1c1e1c22ce891184d390def30a9b8f15f355c05a7bd6e7e7912b571141e01415",
    "height": 100,
    "mediantime": 1550721855,
    "merkleroot": "8570ff4cd3d356be2f9d90333205f9d4b11bd234aa513bf16c53c503f6575718",
    "minerinfo": "",
    "nextblockhash": "0c1a3bddf4686c6be9be8c4be2d3915c542856adac8c7458bd056df83e434eaa",
    "nonce": 0,
    "previousblockhash": "05d6db028304da9c126ea34a2820bfda8a269e2ceb9bfdda73e70632ef13be7f",
    "size": 560,
    "strippedsize": 560,
    "time": 1550721855,
    "tx": [
        {
            "attributes": [
                {
                    "data": "33c33ebc08ea3c1a",
                    "usage": 0
                }
            ],
            "blockhash": "1c1e1c22ce891184d390def30a9b8f15f355c05a7bd6e7e7912b571141e01415",
            "blocktime": 1550721855,
            "confirmations": 296,
            "hash": "8570ff4cd3d356be2f9d90333205f9d4b11bd234aa513bf16c53c503f6575718",
            "locktime": 100,
            "payload": {
                "CoinbaseData": ""
            },
            "payloadversion": 4,
            "programs": [],
            "size": 254,
            "time": 1550721855,
            "txid": "8570ff4cd3d356be2f9d90333205f9d4b11bd234aa513bf16c53c503f6575718",
            "type": 0,
            "version": 0,
            "vin": [
                {
                    "sequence": 4294967295,
                    "txid": "0000000000000000000000000000000000000000000000000000000000000000",
                    "vout": 65535
                }
            ],
            "vout": [
                {
                    "address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
                    "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                    "n": 0,
                    "outputlock": 0,
                    "payload": null,
                    "type": 0,
                    "value": "1.50684931"
                },
                {
                    "address": "EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR",
                    "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                    "n": 1,
                    "outputlock": 0,
                    "payload": null,
                    "type": 0,
                    "value": "1.75799086"
                },
                {
                    "address": "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta",
                    "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                    "n": 2,
                    "outputlock": 0,
                    "payload": null,
                    "type": 0,
                    "value": "1.75799088"
                }
            ],
            "vsize": 254
        }
    ],
    "version": 0,
    "versionhex": "00000000",
    "weight": 2240
}
```

### 2.8 获取原始交易信息

```
./ela-cli info getrawtransaction 17296308c322aee00274da494e0b9a08423b65d170bd2235c3b658f7030fd9b9
```

返回如下：

```
"0902000100133233313431363030303939333439323932373506f9a9deeaf33bde5646567b129c8da3fee08db6210a8d17f359caf6e3b353bf320100ffffffff01ac3c65375d4f0f882a7b76639e3408c1b1fa39731ace92a352e1b650ce35a20100ffffffffe85c7e34e11fd8e1257333d509fc1828f8feef1120b0822940be9c88ee58c31d0100ffffffffdce53502041b2a22a354ffbf03d1c0aed9ff44f731aec4d3a2376c84cbc5696b0100ffffffff8234bf50743e34707a48d9c0cc31ccef29c6f3013b44170ee73835099e3574c60100ffffffffcb1210fe4fbd42cd71c4a5ce46a7862e3e4e557cc5ee6c1d3189208525ecf6de0100ffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a300ca9a3b0000000000000000125bc115b91913c9c6347f0e0e3ba3b75c80b9481100b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3d4d634030000000000000000210d4109bf00e6d782db40ab183491c03cf4d6a37a00000000000141403c72d894b0138348a7640f689b0c4003f1a91969e1b1a1f767303f0fda8226fee42f4b15ac650a8df31a68000fc979036a29dec4383be0571f7bf1bcf3c1cd842321034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16cac"
```

### 2.9 查看交易池

```
./ela-cli info getrawmempool
```

返回如下：

```
[
    {
        "attributes": [
            {
                "data": "35343036373034363031353838313931393731",
                "usage": 0
            }
        ],
        "blockhash": "",
        "blocktime": 0,
        "confirmations": 0,
        "hash": "acbb3c92e36db7d11e81a16e478943edf5cfb5bc7437e7319d3348fd4c7cb2bf",
        "locktime": 0,
        "payload": null,
        "payloadversion": 0,
        "programs": [
            {
                "code": "21034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16cac",
                "parameter": "40ad12383bf739c2f38da527f90b93b82faf9cef103fc8ffed25e79769eb03a7d47c6146f5795aa09636ca855295d9dd016b7c34d2a6804f7e0c13564693ba5872"
            }
        ],
        "size": 494,
        "time": 0,
        "txid": "acbb3c92e36db7d11e81a16e478943edf5cfb5bc7437e7319d3348fd4c7cb2bf",
        "type": 2,
        "version": 9,
        "vin": [
            {
                "sequence": 4294967295,
                "txid": "2575de0d6d323881e7cdb8ede3a13c0ac8a108621be42d66be55860621cfe718",
                "vout": 1
            },
            {
                "sequence": 4294967295,
                "txid": "6c90a60a9e1884211f136de8ebab43a70c7f5c7470490b4031a2426584d08f06",
                "vout": 1
            },
            {
                "sequence": 4294967295,
                "txid": "1162ad86e2f80cfc5507fc018ea03cecdc7f48732d034ac76b6c0b7280fc5f98",
                "vout": 1
            },
            {
                "sequence": 4294967295,
                "txid": "f6c41d3ba2248ee3311aa5680be44e95e295823ac4a6018d76a56de818e481b0",
                "vout": 1
            },
            {
                "sequence": 4294967295,
                "txid": "79fe7592307c44e2794286c2782f1599213eb7fd790f89d39fd8fb0a3e7a40ed",
                "vout": 1
            },
            {
                "sequence": 4294967295,
                "txid": "a9d1030614d73518a1b51555e09d63b38fc8d42cbbb8800abcd0008975144606",
                "vout": 1
            }
        ],
        "vout": [
            {
                "address": "8PT1XBZboe17rq71Xq1CvMEs8HdKmMztcP",
                "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                "n": 0,
                "outputlock": 0,
                "payload": {},
                "type": 0,
                "value": "10"
            },
            {
                "address": "EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR",
                "assetid": "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
                "n": 1,
                "outputlock": 0,
                "payload": {},
                "type": 0,
                "value": "0.53794516"
            }
        ],
        "vsize": 494
    }
]
```



## 3.挖矿

### 3.1 开启cpu挖矿

### 3.2 离散挖矿



## 4.lua脚本

### 运行lua脚本



## 5.回滚

### 回滚区块

