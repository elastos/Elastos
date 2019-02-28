# ela-cli 使用说明

## 1.钱包管理

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

### 2.1 查看节点连接数

### 2.2 查看邻居节点

### 2.3 查看节点状态

### 2.4 查看节点当前高度

### 2.5 查看最高区块hash

### 2.6 查看区块信息

### 2.7 查看交易信息

### 2.8 查看交易池



## 3.挖矿

### 3.1 开启cpu挖矿

### 3.2 离散挖矿



## 4.lua脚本

### 运行lua脚本



## 5.回滚

### 回滚区块

