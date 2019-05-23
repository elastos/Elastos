## General Info

- Prerequisite basic knowledge of docker is expected  
- After starting, the miners will automatically start running and about 25 containers are created
- Pre-mined 900 ELA on Mainchain miner reward address, 100,000 ELA on one mainchain address, 100,000 ELA on another mainchain address, 100,000 ELA on DID sidechain address and 100,000 ELA on Token sidechain address. For more, see [Wallets](#Wallets)
- For the docker images that might be used for connecting to mainnet, testnet, regnet or private net, check out [https://cloud.docker.com/u/cyberrepublic/repository/list](https://cloud.docker.com/u/cyberrepublic/repository/list)

## Tools 

- [build_dockerimages.sh](./tools/build_dockerimages.sh): This shell script automatically builds all the binaries for main chain, all the sidechains, services, etc and then packages them to be run inside docker images and if the flags "-p" and "-l" are set to "yes", the built docker images are automatically pushed to [Cyber Republic Docker Hub](https://cloud.docker.com/u/cyberrepublic/repository/list). Note that you need permission to push to the CR dockerhub but you can still build the images locally if you so choose
- [staging_tools.md](./tools/staging_tools.md): This README file contains all the commands that are used in building the private net from scratch(if that's your cup of tea)
- [copy_dockerdata_host.sh](./tools/copy_dockerdata_host.sh): This script automatically copies the appropriate data folders from the running docker container and saves them with the names "backup" in each corresponding nodes directories. This is a very handy tool when you're building your own private net from scratch and want to save the progress
- [copy_freshdata_docker.sh](./tools/copy_freshdata_docker.sh): This script removes all the previous data from your previously ran docker containers and resets it to use the data that is committed to this github repository for each nodes. This is a very handy tool when you want to purge everything from your docker containers and want to reset the data back to the original data(with preloaded wallets and such)

## Tests

- [action_storeinfoon_didchain.json](./test/action_storeinfoon_didchain.json): This file is used for DID sidechain testing 
- [did_example.json](./test/did_example.json): This file contains info that is produced when you create a new DID. The DID sidechain test that's used later on uses this already created DID to make the process easier
- [register_mainchain-dpos-1.lua](./test/register_mainchain-dpos-1.lua): This script is executed when registering for one of the dpos supernodes locally. Please do not use this on production environment
- [register_mainchain-dpos-2.lua](./test/register_mainchain-dpos-2.lua): This script is executed when registering for one of the dpos supernodes locally. Please do not use this on production environment

## Wallets

These are located in the `wallets` folder:

- `foundation.json` - This is where the genesis block's 33 million ELA is created(Note some ELA have been taken out of this account to other addresses for testing purposes)
- `mainchain-miner-reward.json` - This is where the mining rewards from mainchain go
- `preload/mainchains.json` - This is where the two mainchain addresses are located with 100,000 ELA and 100,000 ELA respectively
- `preload/sidechains.json` - This is where the DID and Token sidechain addresses are located with 100,000 DID ELA and 100,000 TOKEN ELA respectively

## Repos used to build 

- [Elastos.ELA](https://github.com/elastos/Elastos.ELA): release_v0.3.2
- [Elastos.ELA.Arbiter](https://github.com/elastos/Elastos.ELA.Arbiter): release_v0.1.1
- [Elastos.ELA.SideChain.ID](https://github.com/elastos/Elastos.ELA.Sidechain.ID): release_v0.1.2
- [Elastos.ELA.SideChain.Token](https://github.com/elastos/Elastos.ELA.SideChain.Token): release_v0.1.2
- [Elastos.ORG.Wallet.Service](https://github.com/elastos/Elastos.ORG.Wallet.Service): master
- [Elastos.ORG.DID.Service](https://github.com/elastos/Elastos.ORG.DID.Service): master
- [Elastos.ORG.API.Misc](https://github.com/elastos/Elastos.ORG.API.Misc): master

## Containers that are run

### Mainchain nodes

- Normal node 1: 10011-10016
- CRC node 1: 10111-10116
- CRC node 2: 10211-10216
- CRC node 3: 10311-10316
- CRC node 4: 10411-10416
- Elected node 1: 10511-10516
- Elected node 2: 10611-10616

### Arbitrator nodes

- CRC node 1: 20114-20115
- CRC node 2: 20214-20215
- CRC node 3: 20314-20315
- CRC node 4: 20414-20415
- Origin node 1: 20514-20515
- Origin node 2: 20614-20615

### DID Sidechain nodes

- DID sidechain node 1: 30111-30115
- DID sidechain node 2: 30211-30215
- DID sidechain node 3: 30311-30315
- DID sidechain node 4: 30411-30415

### Token Sidechain nodes

- Token sidechain node 1: 40111-40115
- Token sidechain node 2: 40111-40115
- Token sidechain node 3: 40111-40115
- Token sidechain node 4: 40111-40115

### Restful Services

- Wallet Service REST API Portal: 8091
- DID Service REST API Portal: 8092
- MISC Service Mainchain REST API Portal: 9091
- MISC Service DID Sidechain REST API Portal: 9092

### Database Layer

- MYSQL Database: 3307

## How to Run

1. Just run with docker-compose from within the corresponding directory:
    
    If you would like to reset the entire environment with fresh data, do the following. This basically removes all the blockchain data that may have been saved previously and resets it back to block height 510
    ```
    tools/copy_freshdata_docker.sh
    docker-compose up --remove-orphans --build --force-recreate -d
    ```
    For users in China, if you get issues pulling images please refer to this post: https://segmentfault.com/a/1190000016083023

    If you would like to start all the nodes from block 0, do the following. Note that doing this means you won't have the pre-loaded ELAs on the wallet addresses anymore and you need to set up everything yourself
    ```
    sudo tools/copy_freshdata_docker.sh
    docker container prune
    sudo rm -rf ~/.volumes/elastos-privnet
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
    
    You should see at least 915 ELA in the miner wallet:
    ```
    {"Desc":"Success","Error":0,"Result":"915.91409329"}
    ```
    
3. Verify the DID Sidechain is running by checking the pre-loaded wallet:

    ```
    curl http://localhost:30112/api/v1/asset/balances/EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3
    ```    
    
    You should see 100,000 ELA in the DID Sidechain wallet pre-loaded:
    ```
    {"Result":"100000","Error":0,"Desc":"Success"}
    ```

4. Verify the Wallet Service is running by checking the pre-loaded main chain wallet:

    ```
    curl http://localhost:8091/api/1/balance/EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr
    ```    
    
    You should see around 100,000 ELA in the miner wallet:
    ```
    {"result":"94999.99895140","status":200}
    ```
    
5. Verify the DID Service is running by checking the pre-loaded DID sidechain wallet

    ```
    curl http://localhost:8092/api/1/balance/EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3
    ```    
    
    You should see 100005 ELA in the DID Sidechain wallet pre-loaded:
    ```
    {"result":"100005","status":200}
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
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr"}}' http://localhost:10014 
    ```

    Should return
    ```
    {
      "error": null,
      "id": null,
      "jsonrpc": "2.0",
      "result": "95010.81059521"
    }
    ```

    Pre-loaded Main chain Address 2:
    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY"}}' http://localhost:10014
    ```

    Should return
    ```
    {
      "error": null,
      "id": null,
      "jsonrpc": "2.0",
      "result": "95020.74324360"
    }
    ```

    Pre-loaded DID Sidechain Address:
    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3"}}' http://localhost:30114 
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
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EUscMawPCr8uFxKDtVxaq93Wbjm1DdtzeW"}}' http://localhost:40114
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
    curl http://localhost:30112/api/v1/asset/balances/EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3
    ```

    You should now see 10005 ELA in the DID Sidechain wallet:
    ```
    {"result":"100005","status":200}
    ```

8. Verify that the API Misc works [Elastos.ORG.Misc.API](https://github.com/elastos/Elastos.ORG.API.Misc)

    The service for mainchain is running on port 9091 and for DID sidechain is running on port 9092
    
    ```
    curl http://localhost:9091/api/1/history/EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY 
    ```
    
    Should return
    
    ```
    {
      "result": {
        "History": [
          {
            "Address": "EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY",
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
              "EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY",
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
          "Address": "EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY",
          "Block_time": 1557343057,
          "Height": 479
        }
      ],
      "status": 200
    }
    ```

## DPoS Testing

### How to register a supernode
COMING SOON

### How to vote for a supernode

With our private net, there are already two supernodes that have been registered so let's try to use some ELA from pre-loaded mainchain addresses to vote for both of them and check the results after.

Note that the pre-loaded data already has votes so you check the current votes by doing
```
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"listproducers", "params":{"start":"0","limit":2}}' http://localhost:10014
```

Should return
```
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": {
    "producers": [
      {
        "ownerpublickey": "03521eb1f20fcb7a792aeed2f747f278ae7d7b38474ee571375ebe1abb3fa2cbbb",
        "nodepublickey": "0295890a17feb7d5191da656089b5daad83f596edcc491f5c91d025b42955a9f25",
        "nickname": "KP Supernode",
        "url": "www.pachhai.com",
        "location": 112211,
        "active": true,
        "votes": "75000",
        "state": "Activate",
        "registerheight": 418,
        "cancelheight": 0,
        "inactiveheight": 0,
        "illegalheight": 0,
        "index": 0
      },
      {
        "ownerpublickey": "03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007",
        "nodepublickey": "021cfade3eddd057d8ca178057a88c4654b15c1ada7ee9ab65517f00beb6977556",
        "nickname": "Noderators",
        "url": "www.noderators.org",
        "location": 112211,
        "active": true,
        "votes": "50000",
        "state": "Activate",
        "registerheight": 368,
        "cancelheight": 0,
        "inactiveheight": 0,
        "illegalheight": 0,
        "index": 1
      }
    ],
    "totalvotes": "125000",
    "totalcounts": 2
  }
}
```

- Give 140000 votes to Noderators supernode using the address that has 17 million ELA in it
  curl -X POST -H "Content-Type: application/json" -d '{
        "sender":[
            {
                "address":"EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
                "privateKey":"109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"
            }
        ],
        "memo":"Voting for Noderators",
        "receiver":[
            {
                "address":"EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
                "amount":"140000",
                "candidatePublicKeys":["03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007"]
            }
        ]
    }' localhost:8091/api/1/dpos/vote

  Let's see if this new vote is counted towards the supernode after 2 blocks:
  ```
  curl http://localhost:9091/api/1/dpos/producer/03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007
  ```
    
  Should return

  ```
  {
    "result": [
      {
        "Producer_public_key": "03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007",
        "Vote_type": "Delegate",
        "Txid": "07831663f0a30d89a8a41b1cd50a4f67c3d91947bcd3aa0aed271f644dbbc858",
        "Value": "50000",
        "Address": "EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr",
        "Block_time": 1557342957,
        "Height": 473
      },
      {
        "Producer_public_key": "03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007",
        "Vote_type": "Delegate",
        "Txid": "7c6fd446346a4445406ed1e5cd8654846b601cf747f9b197c860aeacea65cbc5",
        "Value": "140000",
        "Address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
        "Block_time": 1557451007,
        "Height": 554
      }
    ],
    "status": 200
  }
  ```

- Give 250,000 votes to KP supernode using the address that has 17 million ELA in it
  curl -X POST -H "Content-Type: application/json" -d '{
        "sender":[
            {
                "address":"EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
                "privateKey":"109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"
            }
        ],
        "memo":"Voting for KP Supernode",
        "receiver":[
            {
                "address":"EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
                "amount":"140000",
                "candidatePublicKeys":["03521eb1f20fcb7a792aeed2f747f278ae7d7b38474ee571375ebe1abb3fa2cbbb"]
            }
        ]
    }' localhost:8091/api/1/dpos/vote

  Let's see if this new vote is counted towards the supernode after 2 blocks:
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
        "Address": "EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY",
        "Block_time": 1557343057,
        "Height": 479
      },
      {
        "Producer_public_key": "03521eb1f20fcb7a792aeed2f747f278ae7d7b38474ee571375ebe1abb3fa2cbbb",
        "Vote_type": "Delegate",
        "Txid": "1a67256af742270e5cafbbad9105dd09815b6bbf037460b7953efe0821854f6a",
        "Value": "140000",
        "Address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
        "Block_time": 1557451230,
        "Height": 576
      }
    ],
    "status": 200
  }
  ```

- We used the same ELA address to vote twice for two different supernodes at two different times. We know that you can only vote for multiple supernodes with the same transaction and can use 1 ELA to vote up to 36 supernodes. However, note that if you just vote for one supernode and then another supernode as a different transaction, only the second vote will count and the first vote will be void. 
  ```
  curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"listproducers", "params":{"start":"0"}}' http://localhost:10014 | jq .
  ```

  Should return:
  ```
  {
    "error": null,
    "id": null,
    "jsonrpc": "2.0",
    "result": {
      "producers": [
        {
          "ownerpublickey": "03521eb1f20fcb7a792aeed2f747f278ae7d7b38474ee571375ebe1abb3fa2cbbb",
          "nodepublickey": "0295890a17feb7d5191da656089b5daad83f596edcc491f5c91d025b42955a9f25",
          "nickname": "KP Supernode",
          "url": "www.pachhai.com",
          "location": 112211,
          "active": true,
          "votes": "215000",
          "state": "Activate",
          "registerheight": 418,
          "cancelheight": 0,
          "inactiveheight": 0,
          "illegalheight": 0,
          "index": 0
        },
        {
          "ownerpublickey": "03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007",
          "nodepublickey": "021cfade3eddd057d8ca178057a88c4654b15c1ada7ee9ab65517f00beb6977556",
          "nickname": "Noderators",
          "url": "www.noderators.org",
          "location": 112211,
          "active": true,
          "votes": "50000",
          "state": "Activate",
          "registerheight": 368,
          "cancelheight": 0,
          "inactiveheight": 0,
          "illegalheight": 0,
          "index": 1
        }
      ],
      "totalvotes": "265000",
      "totalcounts": 2
    }
  }
  ```

## DID Sidechain Testing

### Creating a DID, and Storing/Retrieving Metadata

Generally you will use the DID Service running on port `8092` for this - [https://didservice.readthedocs.io](https://didservice.readthedocs.io).

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

### Retrieving the DID info must be on the Misc.API DID Sidechain - port 9092

Even if you use DID.Service to store DID property, you need to use Misc.API for DID sidechain to retrieve the DID property which should be running on port `9092`.

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

The Elastos.ORG.Wallet.Service currently only supports main chain and DID sidechain so we cannot use it to transfer ELA from main chain to token sidechain so, we'll need to resort to using some command line tools to perform this testing.

1. Download and Clone the repository at: https://github.com/elastos/Elastos.ELA.Client onto your $GOPATH/src/github.com/elastos directory

    ```
    $ echo $GOPATH
    ```

    Should return your $GOPATH dir
    
    ```
    /home/kpachhai/dev
    ```

    Then, clone the repository

    ```
    $ git clone https://github.com/elastos/Elastos.ELA.Client
    ```

    Should return

    ```
    Cloning into 'Elastos.ELA.Client'...
    remote: Enumerating objects: 31, done.
    remote: Counting objects: 100% (31/31), done.
    remote: Compressing objects: 100% (22/22), done.
    remote: Total 1258 (delta 11), reused 12 (delta 9), pack-reused 1227
    Receiving objects: 100% (1258/1258), 227.58 KiB | 3.08 MiB/s, done.
    Resolving deltas: 100% (757/757), done.
    ```

2. Build the ela-cli client

    ```
    $ cd Elastos.ELA.Client/
    $ git checkout dev
    $ rm -rf vendor glide.lock
    $ glide cc && glide update && glide install
    $ make
    ```

3. Configure ela-cli config file

    Create a file called "cli-config.json" and put the following content in that file:

    ```
    {
      "Host": "127.0.0.1:10014",
      "DepositAddress":"XVfmhjxGxBKgzYxyXCJTb6YmaRfWPVunj4"
    }
    ```

    This just means that we're connecting to one of our main chain nodes that's running locally at 127.0.0.1:10014(this is our ela-mainchain-normal-1 docker container that's running our main chain node and port 10336 is HttpJsonPort). The DepositAddress parameter is the deposit address of the sidechain genesis block.

    You can get genesis block hash for token sidechain doing the following:

    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getblockhash","params":{"height":0}}' http://localhost:10044
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
    ./ela-cli wallet -g b569111dfb5e12d40be5cf09e42f7301128e9ac7ab3c6a26f24e77872b9a730e
    ```

    Should return

    ```
    genesis program hash: 0e739a2b87774ef2266a3cabc79a8e1201732fe409cfe50bd4125efb1d1169b5
    genesis address:  XVfmhjxGxBKgzYxyXCJTb6YmaRfWPVunj4
    ```

    So, you plug in the genesis address "XVfmhjxGxBKgzYxyXCJTb6YmaRfWPVunj4" onto the cli-config.json file for "DepositAddress" parameter. This is what allows us to transfer assets from main chain to any of the sidechains. If you want to transfer to DID sidechain, you just plug in the deposit address for the DID sidechain instead.

4. Create a new wallet using ela-cli client for testing purposes

    Note that the ela-cli built using elastos/Elastos.ELA.Client and elastos/Elastos.ELA are not compatible at this point so we'll need to create a new wallet and transfer some ELA to this address and then we can use that new wallet to transfer ELA to our token sidechain address

    ```
    ./ela-cli wallet --create
    ```

    Enter your desired password. I put "elastos" as my password
    
    Should return something like

    ```
    ADDRESS                            PUBLIC KEY                      ------------------------------------------------------------------
    ESKgZtD8BUQT1f4e2RmAvFzcDvjY6Ta8vC 028656f59c88f18bf38f8b3cd85d725fc0ffcf7cd38a5b18e3d2dc623041d2998e
    ------------------------------------------------------------------
    ```

5. Transfer ELA from one of your pre-loaded mainchain wallet to this newly created wallet

    ```
    curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr","privateKey": "a24ee48f308189d46a5f050f326e76779b6508d8c8aaf51a7152b903b9f42f80"}],"receiver": [{"address": "ESKgZtD8BUQT1f4e2RmAvFzcDvjY6Ta8vC","amount": "10100"}]}' localhost:8091/api/1/transfer
    ```

    Should return something like

    ```
    {"result":"dc6089a4bea1e0797e9039bfcb31d41311956c1b0cdd780bbc1764c04558aba6","status":200}
    ```

    Check whether the ELA got transferred successfully

    ```
    ./ela-cli wallet -l
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

6. Transfer ELA from main chain to token sidechain

    ```
    ./ela-cli wallet -t create --from ESKgZtD8BUQT1f4e2RmAvFzcDvjY6Ta8vC --deposit ESKgZtD8BUQT1f4e2RmAvFzcDvjY6Ta8vC --amount 50 --fee 0.0001
    ./ela-cli wallet -t sign -p elastos --file to_be_signed.txn
    ./ela-cli wallet -t send --file ready_to_send.txn
    ```

    Should return the transaction hash if successfull

    ```
    d56c5df3c05af4c583a84d6bf10de3bd6403d456545d4bd462b45b1c74611c3b
    ```

    Wait around 12 blocks so this transaction is confirmed and added to the blockchain. And then check whether the ELA was transferred successfully

    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"ESKgZtD8BUQT1f4e2RmAvFzcDvjY6Ta8vC"}}' http://localhost:40144 
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

### Create a fungible token 

COMING SOON

### Create a non-fungible token

COMING SOON

## Ethereum Sidechain Testing

COMING SOON

## NEO Sidechain Testing

COMING SOON
