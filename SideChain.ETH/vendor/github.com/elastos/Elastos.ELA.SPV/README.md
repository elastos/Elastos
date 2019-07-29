# Elastos SPV

## Summary
Elastos SPV is a SDK of SPV (Simplified Payment Verification) implementation of the Elastos digital currency.
The Elastos SPV SDK is a set of encryption algorithm, peer to peer network and SPV related implementation like bloom filter, merkleblock and util methods.
As an example, this project include a spv wallet implementation located in `spvwallet` folder, it will help you understand how to use this SDK and build your own apps.
The flowing instructions will help you get into the SDK and build up the `spvwallet` sample APP and play with it.

## SDK (Software Development Kit)

1. Account (sdk/account.go)
A ELA standard account is a set of private key, public key, redeem script, program hash and address data.
redeem script is (script content length)+(script content)+(script type),
program hash is the sha256 value of redeem script and converted to ripemd160 format with a (Type) prefix.
address is the base58 format of program hash, which is the string value show up on user interface as account address.
With account, you can get the transfer address or sign transaction etc.

2. AddrFilter (sdk/addrfilter.go)
This is a helper class to filter interested addresses when synchronize transactions
or get cached addresses list to build a bloom filter instead of load addresses from database every time.

3. Blockchain (sdk/blockchain.go)
Blockchain is the database of blocks, also when a new transaction or block commit, Blockchain will verify them with stored blocks.

4. BloomFilter (sdk/bloom.go)
[Bloom filter](https://en.wikipedia.org/wiki/Bloom_filter) is a probabilistic data structure which allows for testing set membership - they can have false positives but not false negatives.
Before synchronize blocks, a `FilterLoad` message need to be sent to the sync peer to filter which transactions should be included in the `merkleblock` message.

5. Crypto (sdk/crypto.go)
This file is the sample code of creating private key, public key and account with ECDSA algorithm.

6. P2P client (sdk/p2pclient.go)
P2P client is a low level interface to interactive with the peer to peer network, you need to creating and responding messages all by yourself except handshake.

7. SPV client (sdk/spvclient.go)
SPV client is a full interface of all SPV messages in the peer to peer network, it will help you create and receive SPV messages and keep heartbeat with the connected peers.

8. SPV service (sdk/spvservice.go)
SPV service is a high level implementation with all SPV logic implemented.
SPV service is extend from SPV client and implement Blockchain and block synchronize on it.
With SPV service, you just need to implement your own DataStore and GetBloomFilter() method, and let other stuff go.

## Build and Run `spvwallet` sample APP

## Build on Mac

### Check OS version

Make sure the OSX version is 16.7+

```shell
$ uname -srm
Darwin 16.7.0 x86_64
```

### Install Go distribution 1.9

Use Homebrew to install Golang 1.9.
```shell
$ brew install go@1.9
```
> If you install older version, such as v1.8, you may get missing math/bits package error when build.

### Setup basic workspace
In this instruction we use ~/dev/src as our working directory. If you clone the source code to a different directory, please make sure you change other environment variables accordingly (not recommended).

```shell
$ mkdir ~/dev/bin
$ mkdir ~/dev/src
```

### Set correct environment variables.

```shell
export GOROOT=/usr/local/opt/go@1.9/libexec
export GOPATH=$HOME/dev
export GOBIN=$GOPATH/bin
export PATH=$GOROOT/bin:$PATH
export PATH=$GOBIN:$PATH
```

### Install Glide

Glide is a package manager for Golang. We use Glide to install dependent packages.

```shell
$ brew install --ignore-dependencies glide
```

### Check Go version and glide version
Check the golang and glider version. Make sure they are the following version number or above.
```shell
$ go version
go version go1.9.2 darwin/amd64

$ glide --version
glide version 0.13.1
```
If you cannot see the version number, there must be something wrong when install.

### Clone source code to `$GOPATH/src/github.com/elastos/` folder
Make sure you are in the folder of `$GOPATH/src/github.com/elastos/`
```shell
$ git clone https://github.com/elastos/Elastos.ELA.SPV.git
```

If clone works successfully, you should see folder structure like $GOPATH/src/github.com/elastos/Elastos.ELA.SPV/makefile

### Glide install

Run `glide update && glide install` to download project dependencies.

### Install bolt and sqlite database
This will make the `make` progress far more fester.
```shell
$ go install github.com/elastos/Elastos.ELA.SPV/vendor/github.com/boltdb/bolt
$ go install github.com/elastos/Elastos.ELA.SPV/vendor/github.com/mattn/go-sqlite3
```

### Make

Run `make` to build the executable files `service` and `ela-wallet`

> `service` is the SPV (Simplified Payment Verification) service running background, communicating with the Elastos peer to peer network and keep updating with the blockchain of Elastos digital currency.

> `ela-wallet` is the command line user interface to communicate with the SPV service, which can create accounts, build sign or send a transaction, or check your account balance.

## Run on Mac

### Set up configuration file
A file named `config.json` should be placed in the same folder with `service` with the parameters as below.
```
{
  "PrintLevel": 4,
  "SeedList": [
    "127.0.0.1:20338"
  ]
}
```
> `PrintLevel` is to control which level of messages can be print out on the console, levels are 0~5, the higher level print out more messages, if set `PrintLevel` to 5 or greater, logs will be save to file.

> `SeedList` is the seed peer addresses in the peer to peer network, SPV service will connect to the peer to peer network through these seed peers.

### Create your wallet
Run `./ela-wallet create` and enter password on the command line tool to create your wallet and master account.
```shell
$ ./ela-wallet create
INPUT PASSWORD:
CONFIRM PASSWORD:
INDEX                            ADDRESS                                                         PUBLIC KEY   TYPE
----- ---------------------------------- ------------------------------------------------------------------ ------
    1 ERpTjzeVnyuCyddRLPK2ednuSK3rdNKjHP 02d790d4021ad89e1c4b0d4b4874467a0bc4100793aed41537e6ee8980efe85c1a MASTER
----- ---------------------------------- ------------------------------------------------------------------ ------
```

### Start SPV service
Run `./service` to start the SPV service
```shell
$ ./service
2018/03/26 23:20:50.995624 [INFO]  PeerManager start
2018/03/26 23:20:50.995804 [INFO]  SPV service started...
2018/03/26 23:20:50.995813 [DEBUG] RPC server started...
...
```

### See account balance
Run `./ela-wallet account -b` to show your account balance.
```shell
$ ./ela-wallet account -b
INDEX                            ADDRESS BALANCE                           (LOCKED)   TYPE
----- ---------------------------------- ------------------------------------------ ------
    1 ERpTjzeVnyuCyddRLPK2ednuSK3rdNKjHP 0                             (0.29299850) MASTER
----- ---------------------------------- ------------------------------------------ ------
    2 EUyNwnAh5SzzTtAPV1HkXzjUEbw2YqKsUM 0                                      (0)    SUB
----- ---------------------------------- ------------------------------------------ ------
```

### Help menu
To see `help` menu, just run `./ela-wallet` or `./ela-wallet -h`
```shell
$ ./ela-wallet
NAME:
   ELASTOS SPV WALLET - command line user interface

USAGE:
   [global option] command [command options] [args]

VERSION:
   6e3e-dirty

COMMANDS:
     create           create wallet
     changepassword   change wallet password
     reset            reset wallet database including transactions, utxos and stxos
     account, a       account [command] [args]
     transaction, tx  use [--create, --sign, --send], to create, sign or send a transaction
     help, h          Shows a list of commands or help for one command

GLOBAL OPTIONS:
   --help, -h     show help
   --version, -v  print the version
```

See sub commands help by input sub command name like `./ela-wallet account`
```shell
$ ./ela-wallet account
NAME:
   ELASTOS SPV WALLET HELP account - account [command] [args]

USAGE:
   ELASTOS SPV WALLET HELP account [command options] [args]

DESCRIPTION:
   commands to create new sub account or multisig account and show accounts balances

OPTIONS:
   --password value, -p value          keystore password
   --list, -l                          list all accounts, including address, public key and type
   --new, -n                           create a new sub account
   --addmultisig value, --multi value  add a multi-sign account with signers public keys
                                       use -m to specify how many signatures are needed to create a valid transaction
                                       by default M is public keys / 2 + 1, which means greater than half
   -m value                            the M value to specify how many signatures are needed to create a valid transaction (default: 0)
   --balance, -b                       show accounts balances
```

## Extra

Sample interface implementations are in `/interface` folder.

### keystore
- Keystore is a file based storage to save the account information, including `Password` `MasterKey` `PrivateKey` etc. in AES encrypted format. Keystore interface is a help to create a keystore file storage and master the accounts within it. The interface methods are listed below.

```
/*
Keystore is a file based storage to save the account information,
including `Password` `MasterKey` `PrivateKey` etc. in AES encrypted format.
Keystore interface is a help to create a keystore file storage and master the accounts within it.
*/
type Keystore interface {
	// Create or open a keystore file
	Open(password string) (Keystore, error)

	// Change the password of this keystore
	ChangePassword(old, new string) error

	// Get the main account
	MainAccount() Account

	// Create a new sub account
	NewAccount() Account

	// Get main account and all sub accounts
	GetAccounts() []Account
}

type Account interface {
	// Create a signature of the given data with this account
	Sign(data []byte) ([]byte, error)

	// Get the public key of this account
	PublicKey() *crypto.PublicKey
}
```

### P2P client
- P2P client is the interface to interactive with the peer to peer network implementation, use this to join the peer to peer network and make communication with other peers.

```
/*
P2P client is the interface to interactive with the peer to peer network,
use this to join the peer to peer network and make communication with other peers.
*/
type P2PClient interface {
	// Start the P2P client
	Start()

	// Handle the version message which includes information of a handshake peer
	HandleVersion(callback func(v *p2p.Version) error)

	// Handle a new peer connect
	PeerConnected(callback func(peer *p2p.Peer))

	// Make a message instance with the given cmd
	MakeMessage(callback func(cmd string) (p2p.Message, error))

	// Handle a message from a connected peer
	HandleMessage(callback func(peer *p2p.Peer, msg p2p.Message) error)

	// Get the peer manager of this P2P client
	PeerManager() *p2p.PeerManager
}
```

### SPV service
- SPV service is the interface to interactive with the SPV (Simplified Payment Verification) service implementation running background, you can register specific accounts that you are interested and receive transaction notifications of these accounts.

```
/*
SPV service is the interface to interactive with the SPV (Simplified Payment Verification)
service implementation running background, you can register specific accounts that you are
interested in and receive transaction notifications of these accounts.
*/
type SPVService interface {
	// Register the account address that you are interested in
	RegisterAccount(address string) error

	// Register the TransactionListener to receive transaction notifications
	// when a transaction related with the registered accounts is received
	RegisterTransactionListener(TransactionListener)

	// After receive the transaction callback, call this method
	// to confirm that the transaction with the given ID was handled
	// so the transaction will be removed from the notify queue
	SubmitTransactionReceipt(txId Uint256) error

	// To verify if a transaction is valid
	// This method is useful when receive a transaction from other peer
	VerifyTransaction(Proof, Transaction) error

	// Send a transaction to the P2P network
	SendTransaction(Transaction) error

	// Start the SPV service
	Start() error
}

/*
Register this listener into the SPVService RegisterTransactionListener() method
to receive transaction notifications.
*/
type TransactionListener interface {
	// Type() indicates which transaction type this listener are interested
	Type() TransactionType

	// Confirmed() indicates if this transaction should be callback after reach the confirmed height,
	// by default 6 confirmations are needed according to the protocol
	Confirmed() bool

	// Notify() is the method to callback the received transaction
	// with the merkle tree proof to verify it
	Notify(Proof, Transaction)
}
```

## License
Elastos SPV wallet source code files are made available under the MIT License, located in the LICENSE file.