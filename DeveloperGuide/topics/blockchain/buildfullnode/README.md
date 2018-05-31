## Build a full node of ELA

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

#### Set correct environment variables

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

#### Clone source code to $GOPATH/src folder

Make sure you are in the folder of $GOPATH/src

```shell
$ git clone https://github.com/elastos/Elastos.ELA.git
```

If clone works successfully, you should see folder structure like $GOPATH/src/Elastos.ELA/makefile

#### Glide install

Run `glide update && glide install` to install depandencies.

#### Make

Run `make` to build files.

If you did not see any error message, congratulations, you have made the ELA full node.

### Run on Mac

- run ./node to run the node program.

### Build on Ubuntu

#### Check OS version

Make sure your ubuntu version is 16.04+

```shell
$ cat /etc/issue
Ubuntu 16.04.3 LTS \n \l
```

#### Install basic tools

```shell
$ sudo apt-get install -y git
```

#### Install Go distribution 1.9

```shell
$ sudo apt-get install -y software-properties-common
$ sudo add-apt-repository -y ppa:gophers/archive
$ sudo apt update
$ sudo apt-get install -y golang-1.9-go
```

> If you install older version, such as v1.8, you may get missing math/bits package error when build.

#### Setup basic workspace

In this instruction we use ~/dev/src as our working directory. If you clone the source code to a different directory, please make sure you change other environment variables accordingly (not recommended).

```shell
$ mkdir ~/dev/bin
$ mkdir ~/dev/src
```

#### Set correct environment variables

```shell
export GOROOT=/usr/lib/go-1.9
export GOPATH=$HOME/dev
export GOBIN=$GOPATH/bin
export PATH=$GOROOT/bin:$PATH
export PATH=$GOBIN:$PATH
```

#### Install Glide

Glide is a package manager for Golang. We use Glide to install dependent packages.

```shell
$ cd ~/dev
$ curl https://glide.sh/get | sh
```

#### Check Go version and glide version

Check the golang and glider version. Make sure they are the following version number or above.

```shell
$ go version
go version go1.9.2 linux/amd64

$ glide --version
glide version v0.13.1
```

If you cannot see the version number, there must be something wrong when install.

#### Clone source code to $GOPATH/src folder

Make sure you are in the folder of $GOPATH/src

```shell
$ git clone https://github.com/elastos/Elastos.ELA.git
```

If clone works successfully, you should see folder structure like $GOPATH/src/Elastos.ELA/makefile

#### Glide install

Run `glide update && glide install` to install depandencies.

#### Make

Run `make` to build files.

If you did not see any error message, congratulations, you have made the ELA full node.

### Run on Ubuntu

- run ./node to run the node program.

### Bootstrap using docker

Alternatively if don't want to build it manually. We also provide a `Dockerfile` to help you (You need have a prepared docker env).

```bash
cd docker
docker build -t ela_node_run .

#start container

docker run -p 20334:20334 -p 20335:20335 -p 20336:20336 -p 20338:20338 ela_node_run
```

> Note: don't using Ctrl-C to terminate the output, just close this terminal and open another.

Now you can access ELA Node's rest api:

```bash
curl http://localhost:20334/api/v1/block/height
```

In the above instruction, we use default configuration file `config.json` in the repository; If you familiar with docker you can change the docker file to use your own ELA Node configuration.

### Config the node

#### In most cases, you only need to modify

- AutoMining、
- PayToAddr、
- Magic、
- SeedList、
- HttpInfoPort、
- HttpRestPort、
- HttpWsPort、
- HttpJsonPort、
- NodePort，
- ActiveNet  //for reduce the mining blocks interval
## Inline Explanation

```javascript
{
  "Configuration": {
    "Magic": 20180312,      //Magic Number：Segregation for different subnet. No matter the port number, as long as the magic number not matching, nodes cannot talk to each others.
    "Version": 23,          //Version number
    "SeedList": [           //SeedList. Other nodes will look up this seed list to connect to any of those seed in order to get all nodes addresses.
      "127.0.0.1:10338",
    ],
    "HttpInfoPort": 10333,  //Local web portal port number. User can go to http://127.0.0.1:10333/info to access the web UI
    "HttpInfoStart": true,  //ture to start the webUI, false to disable
    "HttpRestPort": 10334,  //Restful port number
    "HttpWsPort": 10335,    //Websocket port number
    "WsHeartbeatInterval": 60,
    "HttpJsonPort": 10336,  //RPC port number
    "NodePort": 10338,      //P2P port number
    "PrintLevel": 1,        //Log level. Level 0 is the highest, 6 is the lowest.
    "IsTLS": false,         //TLS connection, true or false
    "CertPath": "./sample-cert.pem",  //Certificate path
    "KeyPath": "./sample-cert-key.pem",
    "CAPath": "./sample-ca.pem",
    "MultiCoreNum": 4,      //Max number of CPU cores to mine ELA
    "MaxTransactionInBlock": 10000, //Max transaction number in each block
    "MaxBlockSize": 8000000,        //Max size of a block
    "PowConfiguration": {
      "PayToAddr": "",              //Pay bonus to this address. Cannot be empty if AutoMining set to "true". 
      "AutoMining": false,          //Start mining automatically? true or false
      "MinerInfo": "ELA",           //No need to change.
      "MinTxFee": 100,              //Minimal mining fee
      "ActiveNet": "MainNet"        //Network type. Choices: MainNet、TestNet、RegNet，RegNet. Mining interval are 120s、10s、1s accordingly. Difficulty factor high to low.
    }
  }
}
```

To learn more, visit [documentation](https://github.com/elastos/Elastos.ELA/blob/master/docs/config.json.md) about config.json

### API of Elastos.ELA

ʻELA Node` uses the `2*334` port to provide the following interface services:

* `/api/v1/node/connectioncount` : Get the number of nodes connected to the current node

   Example:

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

* `/api/v1/block/height` : Get the total height of the node block

* `/api/v1/transactionpool` : Get node transaction pool data

* `/api/v1/restart` : Restart the node server

* `/api/v1/block/hash/<height>` : Get block `hash` according to block `height`

    Example:

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

* `/api/v1/block/details/height/<height>` : Get block details from block `height`

* `/api/v1/block/details/hash/<hash>` : Get block details from block `hash`

* `/api/v1/block/transactions/height/<height>` : Get all trading information from block `height`

* `/api/v1/transaction/<hash>` : Get all transaction information for block based on deal `hash`

* `/api/v1/asset/balances/<addr>` : Get wallet balance based on wallet address

    Example:

    ```bash
    curl http://localhost:20334/api/v1/asset/balances/ES4p9GBXV4n8PayEPyiEmCrjKoRXTfYR4Q

    {
    "Action": "getbalancebyaddr",
    "Desc": "Success",
    "Error": 0,
    "Result": "7696.48402886",
    "Version":"1.0.0"
    }
    ```

* `/api/v1/asset/<hash>` : Asset Query

* `/api/v1/asset/utxos/<addr>` : Get an address for all transactions ʻUTXO`

* `/api/v1/asset/balance/<addr>/<assetid>` : query balance based on address and AssetID

* `/api/v1/asset/utxo/<addr>/<assetid>` : Query UTXO based on address and AssetID

* `/api/v1/transaction` : Create a transaction

    Example:

    ```bash
    #Transaction requires two steps

    #First use the following request method to get RawTransaction
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

    # Then build the following request using the returned rawTx value as RawTransaction

    {
        "method": "decodeRawTransaction",
        "id": 0,
        "params": [{
            "RawTransaction": "02000100142D37323733373430363730363936353637333337015220F787A81709244D9987606E77A74411F61D7E20930924F81A1F4815DEBA2200000000000001B037DB964A231458D2D6FFD5EA18944C4F90E63D547C5D3B9874DF66A4EAD0A30070AE1993A70A000000000021C3B5C32D6FE7CAC86A855276D087C443FB12178B00000000014140E62D5E3E8E14B33377F7EA7301968B81163959A572178CC555F184B2F5239BB683B62E6F178E4C07D6B0D43F780A289488634E4B477197196B8F95581ACA1322232102EE009B86F9377820B1DE396888E7456FDE2554E77E1D9A1AB3360562F1D6FF4BAC"
        }]
    }
    ```

To learn more, please, visit [Elastos Wallet Node API documentation](https://github.com/elastos/Elastos.ELA/blob/master/docs/Elastos_Wallet_Node_API_CN.md)