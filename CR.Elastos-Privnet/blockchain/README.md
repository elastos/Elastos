## General Info

- Prerequisite basic knowledge of docker is expected  
- After starting, the miners will automatically start running and about 25 containers are created
- Pre-mined 1100 ELA on Mainchain miner reward address, 100,000 ELA on one mainchain address, 100,000 ELA on another mainchain address, 100,000 ELA on DID sidechain address and 100,000 ELA on Token sidechain address. For more, see [Wallets](#Wallets)


## Wallets

These are located in the `wallets` folder:

- `foundation.json` - This is where the genesis block's 33 million ELA is created(Note some ELA have been taken out of this account to other addresses for testing purposes)
- `mainchain-miner-reward.json` - This is where the mining rewards from mainchain go
- `preload/mainchains.json` - This is where the two mainchain addresses are located with 100,000 ELA and 100,000 ELA respectively
- `preload/sidechains.json` - This is where the DID and Token sidechain addresses are located with 100,000 DID ELA and 100,000 TOKEN ELA respectively

## Repos used to build 

- [Elastos.ELA](https://github.com/elastos/Elastos.ELA): Apr 12, 2019: release_v0.3.1 a974e079fd6b50f9fa70d93b5647d8a7303dfa08
- [Elastos.ELA.Arbiter](https://github.com/elastos/Elastos.ELA.Arbiter): Apr 4, 2019: release_v0.1.1 758efcdad3266a054bbc823aa87d22cb8343d5e9
- [Elastos.ELA.SideChain.ID](https://github.com/elastos/Elastos.ELA.Sidechain.ID): Apr 12, 2019: release_v0.1.1 fc639ea4e19324f70981c82a4548ef8ee3b4bce5
- [Elastos.ELA.SideChain.Token](https://github.com/elastos/Elastos.ELA.SideChain.Token): Apr 12, 2019: release_v0.1.1 ebd23319c29cfb6c85a1671c39314bd2c45a602d
- [Elastos.ORG.Wallet.Service](https://github.com/elastos/Elastos.ORG.Wallet.Service): Apr 15, 2019: master 47ca5e2ef167680c73da5377f3526477d63e8b1a
- [Elastos.ORG.DID.Service](https://github.com/elastos/Elastos.ORG.DID.Service): Feb 21, 2019: master 8784b182debc4cbd22b607850b261aef7557b8ac
- [Elastos.ORG.API.Misc](https://github.com/elastos/Elastos.ORG.API.Misc): Apr 22, 2019: master d06114b77b789bbd7a769221ff3fa6bddfd8edbe

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

- Origin node 1: 10024-10025
- Origin node 2: 10124-10125
- CRC node 1: 10524-10525
- CRC node 2: 10624-10625
- CRC node 3: 10724-10725
- CRC node 4: 10824-10825

### DID Sidechain nodes

- DID sidechain node 1: 10031-10035
- DID sidechain node 2: 10131-10135
- DID sidechain node 3: 10231-10235
- DID sidechain node 4: 10331-10335

### Token Sidechain nodes

- Token sidechain node 1: 10041-10045
- Token sidechain node 2: 10141-10145
- Token sidechain node 3: 10241-10245
- Token sidechain node 4: 10341-10345

### Restful Services

- Wallet Service REST API Portal: 8091
- DID Service REST API Portal: 8092
- MISC Service Mainchain REST API Portal: 8093
- MISC Service DID Sidechain REST API Portal: 8094

## How to Run

1. Just run with docker-compose from within the corresponding directory:
    
    ```
    docker-compose up --remove-orphans --build --force-recreate -d
    ```
    For users in China, if you get issues pulling images please refer to this post: https://segmentfault.com/a/1190000016083023
    
2. Verify the Mainchain is running by checking the miner reward wallet:

    ```
    curl http://localhost:10012/api/v1/asset/balances/EQ4QhsYRwuBbNBXc8BPW972xA9ANByKt6U
    ```    
    
    You should see at least 1100 ELA in the miner wallet:
    ```
    {"Desc":"Success","Error":0,"Result":"1217.03190898"}
    ```
    
3. Verify the DID Sidechain is running by checking the pre-loaded wallet:

    ```
    curl http://localhost:10032/api/v1/asset/balances/EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3
    ```    
    
    You should see 100,000 ELA in the DID Sidechain wallet pre-loaded:
    ```
    {"Result":"100000","Error":0,"Desc":"Success"}
    ```

4. Verify the Wallet Service is running by checking the pre-loaded main chain wallet:

    ```
    curl http://localhost:8091/api/1/balance/EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr
    ```    
    
    You should see at least 100,000 ELA in the miner wallet:
    ```
    {"result":"100000","status":200}
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
      "result": "17001625.63953213"
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
      "result": "100000"
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
      "result": "100000"
    }
    ```

    Pre-loaded DID Sidechain Address:
    ```
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3"}}' http://localhost:10034 
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
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"EUscMawPCr8uFxKDtVxaq93Wbjm1DdtzeW"}}' http://localhost:10044
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
    curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr","privateKey": "a24ee48f308189d46a5f050f326e76779b6508d8c8aaf51a7152b903b9f42f80"}],"receiver": [{"address": "EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3","amount": "5"}]}' localhost:8091/api/1/cross/m2d/transfer
    ```
    
    You should see the transaction hash returned and a 200 status
    ```
    {"result":"2ac2828b55972cfa63b927f0b42af2c7460e2d618b0df326a6067cbdd2d9147f","status":200}
    ```
    
    After about 12 blocks, you can also see the new ELA on your receiver address, it should be greater than the initial 10000
    ```
    curl http://localhost:10032/api/v1/asset/balances/EKsSQae7goc5oGGxwvgbUxkMsiQhC9ZfJ3
    ```

    You should now see 10005 ELA in the DID Sidechain wallet:
    ```
    {"result":"100005","status":200}
    ```

8. Verify that the API Misc works [Elastos.ORG.Misc.API](https://github.com/elastos/Elastos.ORG.API.Misc)

    The service for mainchain is running on port 8093 and for DID sidechain is running on port 8094
    
    ```
    curl http://localhost:8093/api/1/history/EZzfPQYxAKPR9zSPAG161WsmnucwVqzcLY 
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
    curl http://localhost:8093/api/1/dpos/producer/02fc4aed0eee73aee7915519a596c6c22a1e9509a5ca9763672b03d8e24f2a467b
    ```
    
    Should return
    
    ```
    {
      "result": [
        {
          "Producer_public_key": "02fc4aed0eee73aee7915519a596c6c22a1e9509a5ca9763672b03d8e24f2a467b",
          "Vote_type": "Delegate",
          "Txid": "83a2861cdb27e6feccb6b69dcd85c806797bc8fa14b890a7289926f10fe958ce",
          "Value": "50000",
          "Address": "EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
          "Block_time": 1556077974,
          "Height": 538
        }
      ],
      "status": 200
    }
    ```

## DPoS Testing

### How to register a supernode
COMING SOON

### How to vote for a supernode

With our private net, there are already two supernodes that have been registered so let's try to use some ELA from pre-loaded mainchain addresses to vote for one of them and check the results after

```
curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr","privateKey": "a24ee48f308189d46a5f050f326e76779b6508d8c8aaf51a7152b903b9f42f80"}],"receiver": [{"address": "EPqoMcoHxWMJcV3pCAsGsjkoTdi6DBnKqr","amount": "10000","candidatePublicKeys":["02fc4aed0eee73aee7915519a596c6c22a1e9509a5ca9763672b03d8e24f2a467b"]}]}' localhost:8091/api/1/dpos/vote
```

Should return:

```{"result":"5ab1e0e9d374c58b45bc60c62f34a11adf3933738bd203d9672d69c40d7ec92f","status":200}
```

After about 12 blocks, check whether the new votes are in:
```
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"listproducers", "params":{"start":"0","limit":2}}' http://localhost:10014
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
        "ownerpublickey": "02fc4aed0eee73aee7915519a596c6c22a1e9509a5ca9763672b03d8e24f2a467b",
        "nodepublickey": "021cfade3eddd057d8ca178057a88c4654b15c1ada7ee9ab65517f00beb6977556",
        "nickname": "Noderators",
        "url": "www.noderators.org",
        "location": 61,
        "active": true,
        "votes": "60000",
        "netaddress": "",
        "state": "Activate",
        "registerheight": 244,
        "cancelheight": 0,
        "inactiveheight": 0,
        "illegalheight": 0,
        "index": 0
      },
      {
        "ownerpublickey": "03349e33ed837402a2d54df3c73e7b6146531c96113f5f6eb4dfed3392d0ba227c",
        "nodepublickey": "0295890a17feb7d5191da656089b5daad83f596edcc491f5c91d025b42955a9f25",
        "nickname": "KP Supernode",
        "url": "www.kpsupernode.com",
        "location": 93,
        "active": true,
        "votes": "50000",
        "netaddress": "",
        "state": "Activate",
        "registerheight": 381,
        "cancelheight": 0,
        "inactiveheight": 0,
        "illegalheight": 0,
        "index": 1
      }
    ],
    "totalvotes": "110000",
    "totalcounts": 2
  }
}
```

As you can see, our supernode with public key "02fc4aed0eee73aee7915519a596c6c22a1e9509a5ca9763672b03d8e24f2a467b" now has 60,000 votes compared to the previous 50,000 votes

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

### Retrieving the DID info must be on the Misc.API DID Sidechain - port 8094

Even if you use DID.Service to store DID property, you need to use Misc.API for DID sidechain to retrieve the DID property which should be running on port `8094`.

The API call should be `http://localhost:8094/api/1/did/{did}/{key}`

For example if you stored the property key "clark" above, and assuming the did was `iXxFsEtpt8krhcNbVL7gzRfNqrJdRT4bSw`, then calling

```
curl http://localhost:8094/api/1/did/iXxFsEtpt8krhcNbVL7gzRfNqrJdRT4bSw/clark
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
    "Txid": "4f65869f32ee2ad5397829178ea7ab7b0ef40284ccdac489f3de8db8d5f2000b",
    "Block_time": 1552402320,
    "Height": 1570
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
    curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getreceivedbyaddress","params":{"address":"ESKgZtD8BUQT1f4e2RmAvFzcDvjY6Ta8vC"}}' http://localhost:10044 
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
