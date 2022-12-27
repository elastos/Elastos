## Table of Contents
- [General Info](#general-info)
- [Tools](#tools)
- [Tests](#tests)
- [Wallets](#wallets)
- [Repos used to build](#repos-used-to-build)
- [Containers that are run](#containers-that-are-run)
  - [Mainchain nodes](#mainchain-nodes)
  - [Sidechain nodes](#sidechain-nodes)
  - [Arbitrator nodes](#arbitrator-nodes)
  - [Restful services](#restful-services)
  - [Database layer](#database-layer)
- [How to Run](#how-to-run)
- [DPoS Testing](#dpos-testing)
  - [How to register a supernode](#how-to-register-a-supernode)
  - [How to vote for a supernode](#how-to-vote-for-a-supernode)
  - [Stop your supernode process](#stop-your-supernode-process)
- [DID Sidechain Testing](#did-sidechain-testing)
  - [Creating a DID and Dealing with Metadata](#creating-a-did-and-dealing-with-metadata)
  - [Retrieving DID info](#retrieving-the-did-info)
- [TOKEN Sidechain Testing](#token-sidechain-testing)
  - [Transfer ELA from main chain to Token Sidechain](#transfer-ela-from-main-chain-to-token-sidechain)
- [Ethereum Sidechain Testing](#ethereum-sidechain-testing)
  - [Transfer some ELA from main chain to ETH Sidechain](#transfer-some-ela-from-main-chain-to-eth-sidechain)
  - [Deploy a simple Ethereum smart contract](#deploy-a-simple-ethereum-smart-contract)
- [Stop docker services](#stop-docker-services)

## General Info
- Prerequisite basic knowledge of docker is expected  
- After starting, the miners will automatically start running and about 18 containers are created
- Pre-mined 1000 ELA on Mainchain miner reward address, 3,000,000 ELA on one mainchain address, 3,000,000 ELA on another mainchain address, 100,000 ELA on DID sidechain address, 100,000 ELA on Token sidechain address and 100,000 ELA on ETH sidechain address. For more, see [Wallets](#Wallets)
- For the docker images that might be used for connecting to mainnet, testnet, regnet or private net, check out [https://cloud.docker.com/u/cyberrepublic/repository/list](https://cloud.docker.com/u/cyberrepublic/repository/list)

## Tools 
- [staging_tools.md](./tools/staging_tools.md): This README file contains all the commands that are used in building the private net from scratch(if that's your cup of tea)
- [copy_dockerdata_host.sh](./tools/copy_dockerdata_host.sh): This script automatically copies the appropriate data folders from the running docker container and saves them with the names "backup" in each corresponding nodes directories. This is a very handy tool when you're building your own private net from scratch and want to save the progress
- [copy_freshdata_docker.sh](./tools/copy_freshdata_docker.sh): This script removes all the previous data from your previously ran docker containers and resets it to use the data that is committed to this github repository for each nodes. This is a very handy tool when you want to purge everything from your docker containers and want to reset the data back to the original data(with preloaded wallets and such)

## Tests
- [action_storeinfoon_didchain.json](./test/action_storeinfoon_didchain.json): This file is used for DID sidechain testing 
- [did_example.json](./test/did_example.json): This file contains info that is produced when you create a new DID. The DID sidechain test that's used later on uses this already created DID to make the process easier
- [register_mainchain-dpos-1.lua](./test/register_mainchain-dpos-1.lua): This script is executed when registering for one of the dpos supernodes locally. Please do not use this on production environment
- [cancel_producer_mainchain-dpos-1.lua](./test/cancel_producer_mainchain-dpos-1.lua): This script is executed when cancelling one of the dpos supernodes locally. Please do not use this on production environment
- [return_deposit_mainchain-dpos-1.lua](./test/return_deposit_mainchain-dpos-1.lua): This script is executed when getting the original deposit back for one of the dpos supernodes locally. Please do not use this on production environment
- [register_mainchain-dpos-2.lua](./test/register_mainchain-dpos-2.lua): This script is executed when registering for one of the dpos supernodes locally. Please do not use this on production environment

## Wallets
These are located in the `wallets` folder:

- `foundation.json` - This is where the genesis block's 33 million ELA is created(Note some ELA have been taken out of this account to other addresses for testing purposes)
- `mainchain-miner-reward.json` - This is where the mining rewards from mainchain go
- `preload/mainchains.json` - This is where the two mainchain addresses are located with 3,000,000 ELA and 3,000,000 ELA respectively
- `preload/sidechains.json` - This is where the DID and Token sidechain addresses are located with 100,000 DID ELA, 100,000 TOKEN ELA and 100,000 ETH ELA respectively

## Repos used to build 
- [Elastos.ELA](https://github.com/elastos/Elastos.ELA): v0.4.1
- [Elastos.ELA.Arbiter](https://github.com/elastos/Elastos.ELA.Arbiter): v0.1.2
- [Elastos.ELA.SideChain.ID](https://github.com/elastos/Elastos.ELA.SideChain.ID): v0.1.3
- [Elastos.ELA.SideChain.Token](https://github.com/elastos/Elastos.ELA.SideChain.Token): v0.1.2
- [Elastos.ELA.SideChain.ETH](https://github.com/elastos/Elastos.ELA.SideChain.ETH): v0.0.2
- [Elastos.ORG.Wallet.Service](https://github.com/elastos/Elastos.ORG.Wallet.Service): master
- [Elastos.ORG.SideChain.Service](https://github.com/elastos/Elastos.ORG.SideChain.Service): master
- [Elastos.ORG.API.Misc](https://github.com/elastos/Elastos.ORG.API.Misc): master

## Containers that are run

### Mainchain nodes
- Normal node: 10011-10017
- CRC node 1: 10111-10117
- CRC node 2: 10211-10217
- Elected node 1: 10511-10517
- Elected node 2: 10611-10617

### Sidechain nodes
- DID sidechain node: 30111-30115
- Token sidechain node: 40111-40115
- ETH sidechain node: 60111-60113
- ETH sidechain oracle node: 60114

### Arbitrator nodes
- CRC node 1: 50114-50115
- CRC node 2: 50214-50215
- Origin node 1: 50514-50515
- Origin node 2: 50614-50615

### Restful Services
- Wallet Service REST API Portal: 8091
- Sidechain Service REST API Portal: 8092
- MISC Service Mainchain REST API Portal: 9091
- MISC Service DID Sidechain REST API Portal: 9092

### Database Layer
- MYSQL Database: 3307

## How to Run
1. Just run with docker-compose from within the corresponding directory:
    
    If you would like to reset the entire environment with fresh data, do the following. This basically removes all the blockchain data that may have been saved previously and resets it back to block height 510
    ```
    mkdir -p /data/volumes;
    tools/copy_freshdata_docker.sh;
    docker-compose up --remove-orphans --build --force-recreate -d
    ```
    For users in China, if you get issues pulling images please refer to this post: https://segmentfault.com/a/1190000016083023

    If you would like to start all the nodes from block 0, do the following. Note that doing this means you won't have the pre-loaded ELAs on the wallet addresses anymore and you need to set up everything yourself
    ```
    sudo tools/copy_freshdata_docker.sh;
    docker container prune;
    sudo rm -rf /data/volumes/elastos-privnet;
    docker-compose up --remove-orphans --build --force-recreate -d
    ```

    If you would like to resume the blockchain data from where you last left off, do the following. All the blockchain data is saved to ~/.volumes/elastos-privnet so if you were at block height 1000 previously, it'll continue from there without resetting everything
    ```
    docker-compose up --remove-orphans --build --force-recreate -d
    ```
    
2. Verify the Mainchain is running by checking the miner reward wallet:

    ```
    curl http://localhost:10012/api/v1/asset/balances/EQ4QhsYRwuBbNBXc8BPW972xA9ANByKt6U
    ```    
    
    You should see at least 1005 ELA in the miner wallet:
    ```
    {"Desc":"Success","Error":0,"Result":"1005.60664465"}
    ```
    
3. Verify the DID Sidechain is running by checking the pre-loaded wallet:

    ```
    curl http://localhost:30111/api/v1/asset/balances/EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3
    ```    
    
    You should see 100,000 ELA in the DID Sidechain wallet pre-loaded:
    ```
    {"Result":"100000","Error":0,"Desc":"Success"}
    ```

4. Verify the Wallet Service is running by checking the pre-loaded main chain wallet:

    ```
    curl http://localhost:8091/api/1/balance/EYSLC2mgk5KTLEATK6hRYJ77Umnt1Zv72N
    ```    
    
    You should see around 3,000,000 ELA in this preloaded wallet:
    ```
    {"result":"95046.51050338","status":200}
    ```
    
5. Verify the DID Service is running by checking the pre-loaded DID sidechain wallet

    ```
    curl http://localhost:8092/api/1/balance/EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3
    ```    
    
    You should see 100000 ELA in the DID Sidechain wallet pre-loaded:
    ```
    {"result":"100000","status":200}
    ```

6. Verify that all the appropriate addresses are pre-loaded with ELA in them

    Foundation Address - Main chain:
    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"ENqDYUYURsHpp1wQ8LBdTLba4JhEvSDXEw"}}' http://localhost:10014
    ```

    Should return
    ```
    {
      "error": null,
      "id": null,
      "jsonrpc": "2.0",
      "result": "17001663.81432852"
    }
    ```

    Pre-loaded Main chain Address 1:
    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EYSLC2mgk5KTLEATK6hRYJ77Umnt1Zv72N"}}' http://localhost:10014 
    ```

    Should return
    ```
    {
      "error": null,
      "id": null,
      "jsonrpc": "2.0",
      "result": "95049.05959013"
    }
    ```

    Pre-loaded Main chain Address 2:
    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"ELpKMgPAvvFnRR32AKgYhGkXM1s4wZ6ZjD"}}' http://localhost:10014
    ```

    Should return
    ```
    {
      "error": null,
      "id": null,
      "jsonrpc": "2.0",
      "result": "95069.63333583"
    }
    ```

    Pre-loaded DID Sidechain Address:
    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3"}}' http://localhost:30113 
    ```

    Should return
    ```
    {
      "id": null,
      "jsonrpc": "2.0",
      "result": "100000",
      "error": null
    }  
    ```

    Pre-loaded Token Sidechain Address:
    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EUscMawPCr8uFxKDtVxaq93Wbjm1DdtzeW"}}' http://localhost:40113
    ```

    Should return
    ```
    {
      "id": null,
      "jsonrpc": "2.0",
      "result": {
        "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0": "99999.99990000"
      },
      "error": null
    }
    ```

    Pre-loaded ETH Sidechain Address:
    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"jsonrpc":"2.0","method":"eth_getBalance","params":["0x4505b967d56f84647eb3a40f7c365f7d87a88bc3", "latest"],"id":1}' localhost:60112
    ```
    
    Should return something like:
    ```
    {
      "jsonrpc": "2.0",
      "id": 1,
      "result": "0x152cf383e51ef1920000"
    }
    ```
    0x152cf383e51ef1920000 is 99998900000000000000000 in decimal format which is the unit in wei. This equals to 99998.9 ETH ELA

7. Verify that cross-chain mainchain to sidechain transfers work

    ```
    curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": "EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3","amount": "5"}]}' localhost:8091/api/1/cross/m2d/transfer
    ```
    
    You should see the transaction hash returned and a 200 status
    ```
    {"result":"2ac2828b55972cfa63b927f0b42af2c7460e2d618b0df326a6067cbdd2d9147f","status":200}
    ```
    
    After about 12 blocks, you can also see the new ELA on your receiver address, it should be greater than the initial 10000
    ```
    curl http://localhost:30111/api/v1/asset/balances/EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3
    ```

    You should now see 10005 ELA in the DID Sidechain wallet:
    ```
    {"result":"100005","status":200}
    ```

8. Verify that the API Misc works [Elastos.ORG.Misc.API](https://github.com/elastos/Elastos.ORG.API.Misc)

    The service for mainchain is running on port 9091 and for DID sidechain is running on port 9092
    
    ```
    curl http://localhost:9091/api/1/history/ELpKMgPAvvFnRR32AKgYhGkXM1s4wZ6ZjD 
    ```
    
    Should return
    
    ```
    {
      "result": {
        "History": [
          {
            "Address": "ELpKMgPAvvFnRR32AKgYhGkXM1s4wZ6ZjD",
            "Txid": "22718f7d7499c43bd8f165367d2b19b1898d971b8b9d112deb004199c5db2b45",
            "Type": "income",
            "Value": 10000000000000,
            "CreateTime": 1556077424,
            "Height": 519,
            "Fee": 0,
            "Inputs": [
              "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s"
            ],
            "Outputs": [
              "ELpKMgPAvvFnRR32AKgYhGkXM1s4wZ6ZjD",
              "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s"
            ],
            "TxType": "TransferAsset",
            "Memo": ""
          }
        ],
        "TotalNum": 1
      },
      "status": 200
    }
    ```

    There are two supernodes already registered. Let's check out producer vote stats for one of them:
    ```
    curl http://localhost:9091/api/1/dpos/producer/03521eb1f20fcb7a792aeed2f747f278ae7d7b38474ee571375ebe1abb3fa2cbbb
    ```
    
    Should return
    
    ```
    {
      "result": [
        {
          "Producer_public_key": "03521eb1f20fcb7a792aeed2f747f278ae7d7b38474ee571375ebe1abb3fa2cbbb",
          "Vote_type": "Delegate",
          "Txid": "810910a1578c3e05365c0c57eafa2a23122361199e4220084b4e1ba3827b1c57",
          "Value": "75000",
          "Address": "ELpKMgPAvvFnRR32AKgYhGkXM1s4wZ6ZjD",
          "Block_time": 1557343057,
          "Height": 479
        }
      ],
      "status": 200
    }
    ```

## DPoS Testing

### How to register a supernode
1. Create a directory to work off of:

  This uses release v0.3.5 binary for ela program but if there's a newer version, make sure to grab that instead
  ```
  mkdir -p ~/node/ela;
  cd ~/node/ela;
  wget https://download.elastos.org/elastos-ela/elastos-ela-0.3.5/elastos-ela-v0.3.5-linux-x86_64.tgz;
  tar -xzvf elastos-ela-v0.3.5-linux-x86_64.tgz;
  mv elastos-ela-v0.3.5/ela .;
  mv elastos-ela-v0.3.5/ela-cli .;
  rm -rf elastos-ela-v0.3.5-linux-x86_64.tgz elastos-ela-v0.3.5/
  ```

2. Let's create a new wallet that we will use to register for our supernode so we'll be both an owner and a node

  ```
  ./ela-cli --rpcport 10014 wallet create -p elastos
  ```

  Should return something like:
  ```
  ADDRESS                            PUBLIC KEY                                                        
  ---------------------------------- ------------------------------------------------------------------
  Ec39reRzMTixsYt7yoXkQWDi7kb5dwbj66 036d49dfbb70932b8aea1218beee8dd7aa5e0aafa7a079cb15ba468d74c38a99cf
  ---------------------------------- ------------------------------------------------------------------
  ```

3. Let's send some ELA to this ELA address first because we need 5000 ELA to register for our supernode
  Let's first save our ELA Address in a variable so we can keep on using it
  ```bash
  ELAADDRESS=$(./ela-cli wallet a -p elastos | tail -2 | head -1 | cut -d' ' -f1)
  PUBLICKEY=$(./ela-cli wallet a -p elastos | tail -2 | head -1 | cut -d' ' -f2)
  PRIVATEKEY=$(./ela-cli wallet export -p elastos | tail -2 | head -1 | cut -d' ' -f2)
  # Make sure your info is correct
  echo $ELAADDRESS $PUBLICKEY $PRIVATEKEY
  ```

  Send some ELA to this newly created ELA address
  ```
  curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": '"$ELAADDRESS"',"amount": "6000"}]}' localhost:8091/api/1/transfer
  ```

  Wait for the transaction to confirm(around 6 blocks) and then check your new balance:
  ```
  ./ela-cli --rpcport 10014 wallet b
  ```

  Should return something like:
  ```
  INDEX                            ADDRESS BALANCE                           (LOCKED) 
  ----- ---------------------------------- ------------------------------------------
      0 Ec39reRzMTixsYt7yoXkQWDi7kb5dwbj66 6000                                   (0) 
  ----- ---------------------------------- ------------------------------------------
  ```

4. Add ela configuration file: `config.json`

  ```json
  {
    "Configuration": {
      "Magic": 7630401,
      "DisableDNS": true,
      "PermanentPeers": [
        "127.0.0.1:10016",
        "127.0.0.1:10116",
        "127.0.0.1:10216",
        "127.0.0.1:10516",
        "127.0.0.1:10616"
      ],
      "HttpRestStart": true,
      "EnableRPC": true,
      "PrintLevel": 1,
      "MaxLogsSize": 0,
      "MaxPerLogSize": 0,
      "MinCrossChainTxFee": 10000,
      "FoundationAddress": "ENqDYUYURsHpp1wQ8LBdTLba4JhEvSDXEw",
      "DPoSConfiguration": {
        "EnableArbiter": true,
        "Magic": 7630403,
        "PrintLevel": 1,
        "IPAddress": "127.0.0.1",
        "SignTolerance": 5,
        "MaxLogsSize": 0,
        "MaxPerLogSize": 0,
        "OriginArbiters": [
          "02677bd3dc8ea4a9ab22f8ba5c5348fc1ce4ba5f1810e8ec8603d5bd927b630b3e",
          "0232d3172b7fc139b7605b83cd27e3c6f64fde1e71da2489764723639a6d40b5b9"
        ],
        "CRCArbiters": [
          "0386206d1d442f5c8ddcc9ae45ab85d921b6ade3a184f43b7ccf6de02f3ca0b450",
          "0353197d11802fe0cd5409f064822b896ceaa675ea596287f1e5ce009be7684f08"
        ],
        "NormalArbitratorsCount": 2,
        "CandidatesCount": 24,
        "EmergencyInactivePenalty": 0,
        "MaxInactiveRounds": 20,
        "InactivePenalty": 0,
        "PreConnectOffset": 20
      },
      "CheckAddressHeight": 101,
      "VoteStartHeight": 100,
      "CRCOnlyDPOSHeight": 200,
      "PublicDPOSHeight": 500,
      "RpcConfiguration": {
        "User": "user",
        "Pass": "password",
        "WhiteIPList": [
          "0.0.0.0"
        ]
      }
    }
  }
  ```

5. Run ela node

  ```bash
  # This will run the ./ela program in the background and will pass in
  # the password "elastos" and it sends all the outputted logs to
  # /dev/null and only captures error logs to a file called "output"
  echo elastos | nohup ./ela > /dev/null 2>output &
  ```

6. Modify [./test/register_new_supernode.lua](./test/register_new_supernode.lua) 

  Copy the supernode registration script
  ```
  cp $GOPATH/src/github.com/cyber-republic/elastos-privnet/blockchain/test/register_new_supernode.lua .
  ```

  Get deposit_address:
  ```
  ./ela-cli --rpcport 10014 wallet depositaddr $ELAADDRESS
  ```

  Should output your "deposit_address" that you enter on register_new_supernode.lua script. 
  ```bash
  # Note: Your deposit address will be different
  DoMwtRqQw6oDEgbwvxs7SFg8rk5C8aGxRB
  ```

  ```
  ./ela-cli --rpcport 10014 wallet account -p elastos
  ```

  Should output your public key that you can enter for both "own_publickey" and "node_publickey" for testing purposes

  Finally, also, make sure to change "nick_name", "url" and "location" to your own choosing.

7. Register your supernode

  ```
  ./ela-cli --rpcport 10014 script --file ./register_new_supernode.lua
  ```

  If the transaction is successful, it should say "tx send success" at the end of the output

8. Verify your supernode got registered successfully

  ```
  ./ela-cli --rpcport 10014 info listproducers
  ```

  You should see your new supernode listed there

### How to vote for a supernode
- Give 500 votes to Noderators supernode using the same wallet
  ```
  curl -X POST -H "Content-Type: application/json" -d '{
      "sender":[
          {
              "address":'"$ELAADDRESS"',
              "privateKey":'"$PRIVATEKEY"'
          }
      ],
      "memo":"Voting for Dev Workshop Supernode",
      "receiver":[
          {
              "address":'"$ELAADDRESS"',
              "amount":"500",
              "candidatePublicKeys":['"$PUBLICKEY"']
          }
      ]
  }' localhost:8091/api/1/dpos/vote
  ```

  After some blocks, your vote will be seen. Let's verify this:
  ```
  ./ela-cli --rpcport 10014 info listproducers --rpcuser user --rpcpassword password

  ```

  Should output something like:
  ```
  {
      "producers": [
          {
              "active": true,
              "cancelheight": 0,
              "illegalheight": 0,
              "inactiveheight": 0,
              "index": 0,
              "location": 112211,
              "nickname": "KP Supernode",
              "nodepublickey": "0295890a17feb7d5191da656089b5daad83f596edcc491f5c91d025b42955a9f25",
              "ownerpublickey": "03521eb1f20fcb7a792aeed2f747f278ae7d7b38474ee571375ebe1abb3fa2cbbb",
              "registerheight": 418,
              "state": "Active",
              "url": "www.pachhai.com",
              "votes": "75000"
          },
          {
              "active": true,
              "cancelheight": 0,
              "illegalheight": 0,
              "inactiveheight": 0,
              "index": 1,
              "location": 112211,
              "nickname": "Noderators",
              "nodepublickey": "021cfade3eddd057d8ca178057a88c4654b15c1ada7ee9ab65517f00beb6977556",
              "ownerpublickey": "03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007",
              "registerheight": 368,
              "state": "Active",
              "url": "www.noderators.org",
              "votes": "50000"
          },
          {
              "active": true,
              "cancelheight": 0,
              "illegalheight": 0,
              "inactiveheight": 0,
              "index": 2,
              "location": 112211,
              "nickname": "My new awesome supernode",
              "nodepublickey": "02c1198343700f8c924ab36592066b9e938113a32f1df7fe0aac9e6eb1059faa8d",
              "ownerpublickey": "02c1198343700f8c924ab36592066b9e938113a32f1df7fe0aac9e6eb1059faa8d",
              "registerheight": 570,
              "state": "Active",
              "url": "www.mynewawesomesupernode.com",
              "votes": "500"
          }
      ],
      "totalcounts": 3,
      "totalvotes": "125500"
  }
  ```

  As you can see, our newly created supernode has 500 votes now

### Stop your supernode process
  ```
  kill -9 $(ps aux | grep ela | grep -v "123" | grep -v grep | cut -d' ' -f2);
  cd $GOPATH/src/github.com/cyber-republic/elastos-privnet/blockchain
  ```

## DID Sidechain Testing

### Creating a DID and Dealing with Metadata
Generally you will use the DID Service running on port `8092` for this - [https://didservice.readthedocs.io](https://didservice.readthedocs.io)

See "Create DID" for how to create a DID, you will receive both a did and a private key, store this somewhere.

```
curl http://localhost:8092/api/1/gen/did
```

Should return something like
```
{
  "result": {
    "privateKey": "78F3F61DE57C2058FAB709641EAB8880F2312702896F5599FB4A714EBCF3CFFC",
    "publicKey": "02BDA7DBA5E4E1E24245566AF75E34CC9933FAA99FFFC61081156CC05AE65422E2",
    "publicAddr": "EJrijXpAJmFmn6Xbjdh8TZgAYKS1KsK26N",
    "did": "iXxFsEtpt8krhcNbVL7gzRfNqrJdRT4bSw"
  },
  "status": 200
}
```

NOTE: For your use, a DID has already been created and you can find about it at test/did_example.json

[https://didservice.readthedocs.io/en/latest/api_guide.html#create-did](https://didservice.readthedocs.io/en/latest/api_guide.html#create-did)

Then you can call `POST /api/1/setDidInfo` to store data to this DID. There are two private keys, the outer private key is the private key of the wallet address that is paying for the store data transaction, you can use the ELA stored on the DID sidechain in `wallets/preloaded/sidechains.json`
for this.

We will use the pre-configured DID to store info. Refer to test/action_storeinfoon_didchain.json for what to pass in the body of the request to this API endpoint.

```
cat test/action_storeinfoon_didchain.json
```

Should return
```
{
  "privateKey": "1d5fdc0ad6b0b90e212042f850c0ab1e7d9fafcbd7a89e6da8ff64e8e5c490d2",
  "settings": {
    "privateKey": "78F3F61DE57C2058FAB709641EAB8880F2312702896F5599FB4A714EBCF3CFFC",
    "info": {
      "Tag": "DID Property",
      "Ver": "1.0",
      "Status": 1,
      "Properties": [
        {
          "Key": "clark",
          "Value": "hello,world",
          "Status": 1
        }
      ]
    }
  }
}
```

The inner settings struct is the actual DID to modify, so you will use the private key from `/api/1/gen/did` here to specify that DID. For this example, you can use the file as is.

There is a cost of 10,000 SELA per 1kb on this privatenet, actual cost for the mainnet is not finalized.   

And when you want to send a request to post the above file to the DID sidechain with the key "clark" and value "hello,world", do the following(Save the above excerpt into a file named "action_storeinfoon_didchain.json):
```
curl -H "Content-type: application/json" -d @test/action_storeinfoon_didchain.json http://localhost:8092/api/1/setDidInfo
```

Should return something like
```
{"result":"d1855ef2aea97c47cb52d2862a187e0302edda7f8f84d95420a204f121ccc741","status":200}
```

If you try to set the DID info before letting it be propagated to the block, you might get an error with something like 
```
{
  "result": "double spent UTXO inputs detected, transaction hash: 37393136626431306661653962363231663763396539626530636433653264653535306365313766393632656462663430633839383232383061373761376562, input: cf00f4c5600a5d7ec4f89197a555ba1334e506d3ce4f03b7f98b283765693696, index: 0",
  "status": 10001
}
```

Don't be alarmed. Just wait a couple of minutes and try again.

### Retrieving the DID info
Even if you use DID Sidechain Service to store DID property, you need to use Misc.API for DID sidechain to retrieve the DID property which should be running on port `9092`.

The API call should be `http://localhost:9092/api/1/did/{did}/{key}`

For example if you stored the property key "clark" above, and assuming the did was `iXxFsEtpt8krhcNbVL7gzRfNqrJdRT4bSw`, then calling

```
curl http://localhost:9092/api/1/did/iXxFsEtpt8krhcNbVL7gzRfNqrJdRT4bSw/clark
```

Would return something like
```
{
  "result": {
    "Did": "iXxFsEtpt8krhcNbVL7gzRfNqrJdRT4bSw",
    "Did_status": 1,
    "Public_key": "02BDA7DBA5E4E1E24245566AF75E34CC9933FAA99FFFC61081156CC05AE65422E2",
    "Property_key": "clark",
    "Property_value": "hello,world",
    "Txid": "1d46f57b8e892a1f1df7c7fe757a1b49c7d2d88f53c9d89ba299f02ef2b965c4",
    "Block_time": 1557451466,
    "Height": 367
  },
  "status": 200
}
```

## TOKEN Sidechain Testing

## Transfer ELA from main chain to Token Sidechain
1. Change directory
  ```
  cd $GOPATH/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain
  ```

2. Configure ela-cli config file

    Create a file called "cli-config.json" and put the following content in that file:

    ```
    {
      "Host": "127.0.0.1:10014",
      "DepositAddress":"XVfmhjxGxBKgzYxyXCJTb6YmaRfWPVunj4"
    }
    ```

    This just means that we're connecting to one of our main chain nodes that's running locally at 127.0.0.1:10014(this is our ela-mainchain-normal-1 docker container that's running our main chain node and port 10014 is HttpJsonPort). The DepositAddress parameter is the deposit address of the sidechain genesis block.

    You can get genesis block hash for token sidechain doing the following:

    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getblockhash","params":{"height":0}}' http://localhost:40113
    ```

    Should return

    ```
    {
    "id": null,
    "jsonrpc": "2.0",
    "result": "b569111dfb5e12d40be5cf09e42f7301128e9ac7ab3c6a26f24e77872b9a730e",
    "error": null
    }
    ```

    Or, you can also check out one of the arbitrators config.json file to see what the genesis block hash is for any of the sidechains at ela-arbitrator/node_crc/arbitrator-crc-1/config.json

    And then, plug in the genesis block hash into ela-cli command

    ```
    ./ela-cli-crosschain wallet -g b569111dfb5e12d40be5cf09e42f7301128e9ac7ab3c6a26f24e77872b9a730e
    ```

    Should return

    ```
    genesis program hash: 0e739a2b87774ef2266a3cabc79a8e1201732fe409cfe50bd4125efb1d1169b5
    genesis address:  XVfmhjxGxBKgzYxyXCJTb6YmaRfWPVunj4
    ```

    So, you plug in the genesis address "XVfmhjxGxBKgzYxyXCJTb6YmaRfWPVunj4" onto the cli-config.json file for "DepositAddress" parameter. This is what allows us to transfer assets from main chain to any of the sidechains. If you want to transfer to DID sidechain, you just plug in the deposit address for the DID sidechain instead.

3. Create a new wallet using ela-cli-crosschain client for testing purposes

    Note that the ela-cli-crosschain built using github.com/elastos/Elastos.ELA.Client and github.com/elastos/Elastos.ELA are not compatible at this point so we'll need to create a new wallet and transfer some ELA to this address and then we can use that new wallet to transfer ELA to our token sidechain address

    ```
    ./ela-cli-crosschain wallet --create -p elastos
    ```
    
    Should return something like

    ```
    ADDRESS                            PUBLIC KEY                      ------------------------------------------------------------------
    ESKgZtD8BUQT1f4e2RmAvFzcDvjY6Ta8vC 028656f59c88f18bf38f8b3cd85d725fc0ffcf7cd38a5b18e3d2dc623041d2998e
    ------------------------------------------------------------------
    ```

    Save ELA address, Public key and Private key to a variable so it can be used later
    ```bash
    ELAADDRESS=$(./ela-cli-crosschain wallet -a -p elastos | tail -2 | head -1 | cut -d' ' -f1)
    PUBLICKEY=$(./ela-cli-crosschain wallet -a -p elastos | tail -2 | head -1 | cut -d' ' -f2)
    PRIVATEKEY=$(./ela-cli-crosschain wallet --export -p elastos | tail -2 | head -1 | cut -d' ' -f2)
    # Make sure your info is correct
    echo $ELAADDRESS $PUBLICKEY $PRIVATEKEY
    ```

4. Transfer ELA from the resources wallet to this newly created wallet

    ```
    curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": '"$ELAADDRESS"',"amount": "10100"}]}' localhost:8091/api/1/transfer
    ```

    Should return something like

    ```
    {"result":"dc6089a4bea1e0797e9039bfcb31d41311956c1b0cdd780bbc1764c04558aba6","status":200}
    ```

    Check whether the ELA got transferred successfully

    ```
    ./ela-cli-crosschain wallet -l
    ```

    Should return

    ```
    354 / 354 [============================================] 100.00% 8s
    2 / 2 [================================================] 100.00% 0s
    INDEX|ADDRESS|BALANCE|(LOCKED)|TYPE
    ------------------------------------------ 
    1|ESKgZtD8BUQT1f4e2RmAvFzcDvjY6Ta8vC|10100|(0)|MASTER
    ------------------------------------------
    ```

5. Transfer ELA from main chain to token sidechain

    ```
    ./ela-cli-crosschain wallet -t create --from $ELAADDRESS --deposit ESKgZtD8BUQT1f4e2RmAvFzcDvjY6Ta8vC --amount 50 --fee 0.0001;
    ./ela-cli-crosschain wallet -t sign -p elastos --file to_be_signed.txn;
    ./ela-cli-crosschain wallet -t send --file ready_to_send.txn;
    ```

    Should return the transaction hash if successfull

    ```
    d56c5df3c05af4c583a84d6bf10de3bd6403d456545d4bd462b45b1c74611c3b
    ```

    Wait around 12 blocks so this transaction is confirmed and added to the blockchain. And then check whether the ELA was transferred successfully

    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"ESKgZtD8BUQT1f4e2RmAvFzcDvjY6Ta8vC"}}' http://localhost:40113
    ```

    Should return

    ```
    {
    "id": null,
    "jsonrpc": "2.0",
    "result": {
      "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0": "49.99990000"
    },
    "error": null
    }
    ```

    As you can see, we now have around 50 ELA that was transferred to our token sidechain address

7. Remove all the files that are no longer needed
  ```
  rm -f cli-config.json keystore.dat ready_to_send.txn to_be_signed.txn wallet.db;
  ```

### Create a fungible token 

COMING SOON

### Create a non-fungible token

COMING SOON

## Ethereum Sidechain Testing

### Transfer some ELA from main chain to ETH Sidechain
1. Change directory
  ```
  cd $GOPATH/src/github.com/cyber-republic/elastos-privnet/blockchain/ela-mainchain
  ```

2. Configure ela-cli config file

    Create a file called "cli-config.json" and put the following content in that file:

    ```
    {
      "Host": "127.0.0.1:10014",
      "DepositAddress":"XZyAtNipJ7fdgBRhdzCoyS7A3PDSzR7u98"
    }
3. Create a new wallet using ela-cli-crosschain client for testing purposes

    ```
    ./ela-cli-crosschain wallet --create -p elastos
    ```

    Save ELA address, Public key and Private key to a variable so it can be used later
    ```bash
    ELAADDRESS=$(./ela-cli-crosschain wallet -a -p elastos | tail -2 | head -1 | cut -d' ' -f1)
    PUBLICKEY=$(./ela-cli-crosschain wallet -a -p elastos | tail -2 | head -1 | cut -d' ' -f2)
    PRIVATEKEY=$(./ela-cli-crosschain wallet --export -p elastos | tail -2 | head -1 | cut -d' ' -f2)
    # Make sure your info is correct
    echo $ELAADDRESS $PUBLICKEY $PRIVATEKEY
    ```

4. Transfer ELA from the resources wallet to this newly created wallet

    ```
    curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s","privateKey": "109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"}],"receiver": [{"address": '"$ELAADDRESS"',"amount": "100000"}]}' localhost:8091/api/1/transfer
    ```

    Check whether the ELA got transferred successfully

    ```
    ./ela-cli-crosschain wallet -l
    ```
5. Transfer ELA from main chain to eth sidechain

    ```
    ./ela-cli-crosschain wallet -t create --from $ELAADDRESS --deposit 0x4505b967d56f84647eb3a40f7c365f7d87a88bc3 --amount 99999 --fee 0.1;
    ./ela-cli-crosschain wallet -t sign -p elastos --file to_be_signed.txn;
    ./ela-cli-crosschain wallet -t send --file ready_to_send.txn;
    ```
6. Check eth balance:

  ```
  curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"jsonrpc":"2.0","method":"eth_getBalance","params":["0x4505b967d56f84647eb3a40f7c365f7d87a88bc3", "latest"],"id":1}' localhost:60112
  ```

  Should return something like:
  ```
  {
    "jsonrpc": "2.0",
    "id": 1,
    "result": "0x152cf383e51ef1920000"
  }
  ```
  0x152cf383e51ef1920000 is 99998900000000000000000 in decimal format which is the unit in wei. This equals to 99998.9 ETH ELA

## Deploy a simple Ethereum Smart Contract
1. Install Truffle Box
  We're going to be using Truffle Box that will help us in deploying our smart contract to our private ethereum sidechain.

  First make sure to install NodeJS, NPM, Truffle Box and Solidity compiler
  ```
  cd test/eth_smart_contracts;
  sudo apt-get install nodejs npm;
  npm install -g truffle solc;
  ```

2. Compile Eth Smart Contracts
  ```
  truffle compile;
  ```
  Should return something like:
  ```
  Compiling your contracts...
  ===========================
  > Compiling ./contracts/HelloWorld.sol
  > Compiling ./contracts/Migrations.sol
  > Compiling ./contracts/StoreNumber.sol
  > Artifacts written to /home/kpachhai/dev/src/github.com/cyber-republic/developer-workshop/2019-09-04/smart_contracts/build/contracts
  > Compiled successfully using:
    - solc: 0.5.1+commit.c8a2cb62.Emscripten.clang
  ```

3. Unlock our account 
  We're going to unlock our ETH account that is already pre-loaded with some ETH/ELASC. It's address is 0x4505b967d56f84647eb3a40f7c365f7d87a88bc3

  ```
  curl -X POST -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"jsonrpc":"2.0","method":"personal_unlockAccount","params":["0x4505b967d56f84647eb3a40f7c365f7d87a88bc3", "elastos-privnet", 3600],"id":67}' http://localhost:60112
  ```
  Should return
  ```
  {"jsonrpc":"2.0","id":67,"result":true}
  ```

4. Migrate Eth Smart Contracts
  ```
  truffle migrate --network develop;
  ```
  Should return something like
  ```
  Compiling your contracts...
  ===========================
  > Everything is up to date, there is nothing to compile.


  Migrations dry-run (simulation)
  ===============================
  > Network name:    'develop-fork'
  > Network id:      3
  > Block gas limit: 0x7a1200


  1_initial_migration.js
  ======================

    Deploying 'Migrations'
    ----------------------
    > block number:        75741
    > block timestamp:     1567521542
    > account:             0x285f996244AA936E1c54Bcf77d5e253790614Af5
    > balance:             0.10900768
    > gas used:            268300
    > gas price:           2 gwei
    > value sent:          0 ETH
    > total cost:          0.0005366 ETH

    -------------------------------------
    > Total cost:           0.0005366 ETH


  2_deploy_HelloWorld.js
  ======================

    Deploying 'HelloWorld'
    ----------------------
    > block number:        75743
    > block timestamp:     1567521542
    > account:             0x285f996244AA936E1c54Bcf77d5e253790614Af5
    > balance:             0.108485426
    > gas used:            234099
    > gas price:           2 gwei
    > value sent:          0 ETH
    > total cost:          0.000468198 ETH

    -------------------------------------
    > Total cost:         0.000468198 ETH


  3_deploy_StoreNumber.js
  =======================

    Deploying 'StoreNumber'
    -----------------------
    > block number:        75745
    > block timestamp:     1567521543
    > account:             0x285f996244AA936E1c54Bcf77d5e253790614Af5
    > balance:             0.108203052
    > gas used:            114159
    > gas price:           2 gwei
    > value sent:          0 ETH
    > total cost:          0.000228318 ETH

    -------------------------------------
    > Total cost:         0.000228318 ETH


  Summary
  =======
  > Total deployments:   3
  > Final cost:          0.001233116 ETH


  Starting migrations...
  ======================
  > Network name:    'develop'
  > Network id:      3
  > Block gas limit: 0x7a1200


  1_initial_migration.js
  ======================

    Deploying 'Migrations'
    ----------------------
    > transaction hash:    0x9428fca370b41ff5d6468057753675f2a14aa1aed6671194fba38a50cf559b59
    > Blocks: 0            Seconds: 12
    > contract address:    0x44bd7606C53a53088fDeDE4Bab294d3eD9AcB43d
    > block number:        75741
    > block timestamp:     1567521557
    > account:             0x285f996244AA936E1c54Bcf77d5e253790614Af5
    > balance:             0.10387828
    > gas used:            283300
    > gas price:           20 gwei
    > value sent:          0 ETH
    > total cost:          0.005666 ETH


    > Saving migration to chain.
    > Saving artifacts
    -------------------------------------
    > Total cost:            0.005666 ETH


  2_deploy_HelloWorld.js
  ======================

    Deploying 'HelloWorld'
    ----------------------
    > transaction hash:    0x9f51eef8f8d635b1bb1847122ff9f60bb035a1a9e603f16dcd497ba5ea725550
    > Blocks: 0            Seconds: 12
    > contract address:    0x0CF5E37FB86A19E14Ddf305a6B65754dB8dB2F22
    > block number:        75743
    > block timestamp:     1567521587
    > account:             0x285f996244AA936E1c54Bcf77d5e253790614Af5
    > balance:             0.09835574
    > gas used:            234099
    > gas price:           20 gwei
    > value sent:          0 ETH
    > total cost:          0.00468198 ETH


    > Saving migration to chain.
    > Saving artifacts
    -------------------------------------
    > Total cost:          0.00468198 ETH


  3_deploy_StoreNumber.js
  =======================

    Deploying 'StoreNumber'
    -----------------------
    > transaction hash:    0xfe7f7fede1aba3e42f9b5d878f5e7a7f63ed94280f745d397bdf28d243e704c4
    > Blocks: 0            Seconds: 12
    > contract address:    0x77715e313730a64Ff6a7C8430a4dBe1C90c6463e
    > block number:        75745
    > block timestamp:     1567521617
    > account:             0x285f996244AA936E1c54Bcf77d5e253790614Af5
    > balance:             0.095532
    > gas used:            114159
    > gas price:           20 gwei
    > value sent:          0 ETH
    > total cost:          0.00228318 ETH


    > Saving migration to chain.
    > Saving artifacts
    -------------------------------------
    > Total cost:          0.00228318 ETH


  Summary
  =======
  > Total deployments:   3
  > Final cost:          0.01263116 ETH
  ```

5. Enter the truffle console
  ```
  truffle console --network develop;
  ```

6. Execute HelloWorld.sol smart contract on testnet Ethereum sidechain
  First, let's see what message is currently set 
  ```
  truffle(develop)> var first_contract
  truffle(develop)> HelloWorld.deployed().then(function(instance) { first_contract = instance; })
  truffle(develop)> first_contract.message.call()
  ```
  Should return
  ```
  ''
  ```
  This is because the function Hello() hasn't been called yet.

  Let's now call the function to set its value
  ```
  truffle(develop)> first_contract.Hello()
  truffle(develop)> first_contract.message.call()
  ```
  Should return
  ```
  'Hello World!'
  ```
  Success!

7. Execute StoreNumber.sol smart contract on testnet Ethereum sidechain
  First, let's see what number is currently stored
  ```
  truffle(develop)> var second_contract
  truffle(develop)> StoreNumber.deployed().then(function(instance) { second_contract = instance; })
  truffle(develop)> second_contract.get()
  ```
  Should return
  ```
  BN {
    negative: 0,
    words: [ 0, <1 empty item> ],
    length: 1,
    red: null }
  ```

  Let's now change the value of this item
  ```
  truffle(develop)> second_contract.set(5)
  truffle(develop)> second_contract.get()
  ```
  Now, this should return
  ```
  BN {
    negative: 0,
    words: [ 5, <1 empty item> ],
    length: 1,
    red: null }
  ```
  As you can see, we successfully set the value from 0 to 5

## Stop docker services
  ```
  cd $GOPATH/src/github.com/cyber-republic/elastos-privnet/blockchain && docker-compose down
  ```

## How to run elastos private net in kubernetes
Prerequisites: Install kubernetes and minikube
```
cd kubectl;
sudo minikube start;
sudo sudo chown -R $USER $HOME/.kube $HOME/.minikube;
kubectl apply -R -f .;
```
NOTE: Kubernetes runs containers in a cluster but each "app"(eg. mainchain node, did sidechain node, etc) run in different IP addresses and different ports. View the services to check out their IPs and ports they run on.

Check info about your kubernetes cluster:
```
minikube service list;
kubectl describe services service_name;
kubectl -n default get deployment;
kubectl -n default get pods;
kubectl get svc;
kubectl logs -f pod_name;
kubectl describe pod pod_name;
kubectl exec -it pod_name sh
```

How to stop kubernetest cluster:
```
kubectl -n default delete pod,svc --all;
mnikube stop
minikube delete
```