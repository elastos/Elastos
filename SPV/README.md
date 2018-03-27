# Elastos SPV wallet

## Summary
Elastos SPV wallet is a SPV (Simplified Payment Verification) wallet implementation of the Elastos digital currency, using command line as the user interface.

## Build
##### 1. Download source code to the following path `$GOPATH/src/`
```
$ git clone https://github.com/elastos/Elastos.SPVWallet.git
```

##### 2. Go into `$GOPATH/src/SPVWallet` and run `make install`, this will automatically download and install project dependencies.

##### 3. Run `make` to build the executable files `service` and `ela-wallet`

### Executable files

- `service` is the SPV (Simplified Payment Verification) service running background, communicating with the Elastos peer to peer network and keep updating with the blockchain of Elastos digital currency.

- `ela-wallet` is the command line user interface to communicate with the SPV service, witch can create accounts, build sign or send a transaction, or check your account balance.

## Usage
##### 1. Set up configuration file `config.json`
```
{
  "Magic": 1234567,
  "SeedList": [
    "127.0.0.1:20338"
  ]
}
```
`Magic` is the identify number of the peer to peer network, peers in the same network should be using the same Magic number.

`SeedList` is the seed peer addresses in the peer to peer network, SPV service will connect to the peer to peer network through these seed peers.

##### 2. Run `./ela-wallet create` and enter password on the command line tool to create your wallet and master account.
```
INPUT PASSWORD:
CONFIRM PASSWORD:
INDEX                            ADDRESS                                                         PUBLIC KEY   TYPE
----- ---------------------------------- ------------------------------------------------------------------ ------
    1 ERpTjzeVnyuCyddRLPK2ednuSK3rdNKjHP 02d790d4021ad89e1c4b0d4b4874467a0bc4100793aed41537e6ee8980efe85c1a MASTER
----- ---------------------------------- ------------------------------------------------------------------ ------
```
##### 3. Run `./service` to start the SPV service
```
2018/03/26 23:20:50.995624 [INFO]  PeerManager start
2018/03/26 23:20:50.995804 [INFO]  SPV service started...
2018/03/26 23:20:50.995813 [DEBUG] RPC server started...
...
```
##### 4. Run `./ela-wallet account -b` to show your account balance.
```
INDEX                            ADDRESS BALANCE        (LOCKED)
----- ---------------------------------- -----------------------
    1 ERpTjzeVnyuCyddRLPK2ednuSK3rdNKjHP 0                   (0)
----- ---------------------------------- -----------------------
```
##### 5. To see `help` menu, just run `./ela-wallet` or `./ela-wallet -h`
```
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
##### 6. Show sub commands help by input sub command name like `./ela-wallet account`
```
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
                                       by default M is public keys / 2 + 1, witch means greater than half
   -m value                            the M value to specify how many signatures are needed to create a valid transaction (default: 0)
   --balance, -b                       show accounts balances
```

## Develop
##### This project can be used as a reference to build your own apps, it provides several interfaces for developers.
##### 1. keystore
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
##### 2. P2P client
- P2P client is the interface to interactive with the peer to peer network implementation, use this to join the peer to peer network and make communication with other peers.

```
/*
P2P client is the interface to interactive with the peer to peer network implementation,
use this to join the peer to peer network and make communication with other peers.
*/
type P2PClient interface {
	// Start the P2P client
	Start()

	// Handle the version message witch includes information of a handshake peer
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
##### 3. SPV service
- SPV service is the interface to interactive with the SPV (Simplified Payment Verification) service implementation running background, you can register specific accounts that you are interested in and receive transaction notifications of these accounts.

```
/*
SPV service is the interface to interactive with the SPV (Simplified Payment Verification)
service implementation running background, you can register specific accounts that you are
interested in and receive transaction notifications of these accounts.
*/
type SPVService interface {
	// Register the account address that you are interested in
	RegisterAccount(address string) error

	// Register the callback method to receive transaction notifications
	// when a transaction related with the registered accounts is confirmed
	// The merkle tree proof and the transaction will be callback
	OnTransactionConfirmed(func(db.Proof, tx.Transaction))

	// After receive the transaction callback, call this method
	// to confirm that the transaction with the given ID was handled
	// so the transaction will be removed from the notify queue
	SubmitTransactionReceipt(txId core.Uint256) error

	// To verify if a transaction is valid
	// This method is useful when receive a transaction from other peer
	VerifyTransaction(db.Proof, tx.Transaction) error

	// Send a transaction to the P2P network
	SendTransaction(tx.Transaction) error

	// Start the SPV service
	Start() error
}
```

## License
Elastos SPV wallet source code files are made available under the MIT License, located in the LICENSE file.