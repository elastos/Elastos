## Create your ELA wallet app

[Source code](https://github.com/elastos/Elastos.ELA.SPV/tree/master/spvwallet)

### Build on Mac

#### Check OS version

Make sure the OSX version is 16.7+

```shell
$ uname -srm
Darwin 16.7.0 x86_64
```

#### Install Go distribution 1.9

Use Homebrew to install Golang 1.9.
```shell
$ brew install go@1.9
```
> If you install older version, such as v1.8, you may get missing math/bits package error when build.

#### Setup basic workspace
In this instruction we use ~/dev/src as our working directory. If you clone the source code to a different directory, please make sure you change other environment variables accordingly (not recommended).

```shell
$ mkdir ~/dev/bin
$ mkdir ~/dev/src
```

#### Set correct environment variables.

```shell
export GOROOT=/usr/local/opt/go@1.9/libexec
export GOPATH=$HOME/dev
export GOBIN=$GOPATH/bin
export PATH=$GOROOT/bin:$PATH
export PATH=$GOBIN:$PATH
```

#### Install Glide

Glide is a package manager for Golang. We use Glide to install dependent packages.

```shell
$ brew install --ignore-dependencies glide
```

#### Check Go version and glide version
Check the golang and glider version. Make sure they are the following version number or above.
```shell
$ go version
go version go1.9.2 darwin/amd64

$ glide --version
glide version 0.13.1
```
If you cannot see the version number, there must be something wrong when install.

#### Clone source code to `$GOPATH/src/github.com/elastos/` folder
Make sure you are in the folder of `$GOPATH/src/github.com/elastos/`
```shell
$ git clone https://github.com/elastos/Elastos.ELA.SPV.git
```

If clone works successfully, you should see folder structure like $GOPATH/src/github.com/elastos/Elastos.ELA.SPV/makefile

#### Glide install

Run `glide update && glide install` to download project dependencies.

#### Install bolt and sqlite database
This will make the `make` progress far more fester.
```shell
$ go install github.com/elastos/Elastos.ELA.SPV/vendor/github.com/boltdb/bolt
$ go install github.com/elastos/Elastos.ELA.SPV/vendor/github.com/mattn/go-sqlite3
```

#### Make

Run `make` to build the executable files `service` and `ela-wallet`

> `service` is the SPV (Simplified Payment Verification) service running background, communicating with the Elastos peer to peer network and keep updating with the blockchain of Elastos digital currency.

> `ela-wallet` is the command line user interface to communicate with the SPV service, which can create accounts, build sign or send a transaction, or check your account balance.

### Run on Mac

#### Set up configuration file
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

#### Create your wallet
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

#### Start SPV service
Run `./service` to start the SPV service
```shell
$ ./service
2018/03/26 23:20:50.995624 [INFO]  PeerManager start
2018/03/26 23:20:50.995804 [INFO]  SPV service started...
2018/03/26 23:20:50.995813 [DEBUG] RPC server started...
...
```

#### See account balance
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

#### Help menu
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