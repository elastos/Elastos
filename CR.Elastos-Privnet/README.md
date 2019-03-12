# Elastos Local PrivateNet - v0.1

## This is official but unsupported and ALPHA version

These executables are built from the private repo [https://github.com/ClarenceL/ela-privnet-staging](https://github.com/ClarenceL/ela-privnet-staging) 
which we use to compile the executables into the right environment, only the executables can be found here.

#### These executables are purpose built for the docker images herein, if you want to use them directly you may need to build from source

*If you are a core developer and need to update the executable please contact me for access.*

## Building from Source

Basically we both don't advise this and although most repos are public, some are still private as we do internal security audits.
Therefore you wouldn't be able to build it anyway unless you are a partner and request access.

Here's a taste of how this build process looks from scratch [https://www.cyberrepublic.org/experimental-docs](https://www.cyberrepublic.org/experimental-docs) 

## General Info

- Prerequisite basic knowledge of docker is expected  
- After running the miners will automatically start running, about 6 containers are created so expect some performance hit to your computer
- Pre-mined 500 ELA on Mainchain - see [Wallets](#Wallets)


## Wallets

These are located in the `wallets` folder:

- `foundation.json` - This is where the genesis block's 33 million ELA is created
- `mainchain-miner-reward.json` - This is where the mining rewards from mainchain go
- `sidechain-did-preloaded.json` - Pre-loaded transferred ELA from the mainchain to this DID sidechain wallet 
- `sidechain-token-preloaded.json` - Pre-loaded transferred ELA from the mainchain to this Token sidechain wallet 

## How to Run

1. Download and install Docker - this has been tested on the versions:
    - Docker Engine 18.09.2
    - Docker Compose 1.23.2
    - Docker Machine 0.16.1

2. The following are the GIT SHAs we're using for each project:
    - Elastos.ELA: tag v0.2.2 3f1e5e6435468d735f7fcd12e8d0706832c6ed31
    - Elastos.ELA.SideChain.ID: tag v0.0.2 07599ad42ad32315c692c61b1ab340341080d08f
    - Elastos.ELA.SideChain.Token: release_v0.0.1 2c2138019d5946e80e4fc909227e5e85a7cf19d2
    - Elastos.ELA.Arbiter: tag v0.0.3 8970efe9fec3d77f4d4bab6c19505b33cdb11dec
    - Elastos.ORG.Wallet.Service: master 6f079a98421656a9286d87127049eb75f49df277
    - Elastos.ORG.DID.Service: master 8784b182debc4cbd22b607850b261aef7557b8ac
    - Elastos.ORG.API.Misc: master 78c2c6a621cf088dacbce6f40e0080b873719aee
    - Elastos.NET.Hive.IPFS: dev-master 6a5e24032b0d0c79e06a103f1b4078a648fa0e2e
    - Elastos.NET.Hive.Cluster: dev-master 390b912df5cf496b7c9e6e5d3f6e0fb897a76db1
    
3. Check your ports, here are the following ports these containers expect available:
    - Mainchain: 20333-20338 - [http://github.com/elastos/Elastos.ELA](http://github.com/elastos/Elastos.ELA)
    - DID Sidechain: 20604-20608 - [http://github.com/elastos/Elastos.ELA.SideChain.DID](http://github.com/elastos/Elastos.ELA.SideChain.DID)
    - Token Sidechain: 20614-20618 - [http://github.com/elastos/Elastos.ELA.SideChain.Token](http://github.com/elastos/Elastos.ELA.SideChain.Token)
    - Wallet Service: 8091 - [https://walletservice.readthedocs.io](https://walletservice.readthedocs.io)
    - DID Service: 8092 - [https://didservice.readthedocs.io](https://didservice.readthedocs.io)
    - Misc API: 8093-8094 - [https://github.com/elastos/Elastos.ORG.API.Misc](https://github.com/elastos/Elastos.ORG.API.Misc)
    - HIVE IPFS: 38080 - [http://github.com/elastos/Elastos.NET.Hive.IPFS](http://github.com/elastos/Elastos.NET.Hive.IPFS)
    - HIVE Cluster: 9094-9095 - [http://github.com/elastos/Elastos.NET.Hive.Cluster](http://github.com/elastos/Elastos.NET.Hive.Cluster)
    
    ##### Note: DID Service's - `getDidInfo` method is currently broken, but available on the Misc API - port 8094
    
4. Just run with docker-compose from within the directory:
    
    ```
    docker-compose up --remove-orphans --build --force-recreate -d
    ```
    For users in China, if you get issues pulling images please refer to this post: https://segmentfault.com/a/1190000016083023
    
5. Verify the Mainchain is running by checking the miner reward wallet:

    ```
    curl http://localhost:20334/api/v1/asset/balances/EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE
    ```    
    
    You should see at least 825 ELA in the miner wallet:
    ```
    {"Desc":"Success","Error":0,"Result":"838.86741124"}
    ```
    
6. Verify the DID Sidechain is running by checking the pre-loaded wallet:

    ```
    curl http://localhost:20604/api/v1/asset/balances/EJWT3HbQWXNZk9gDwvGJwXdvv87qkdRkhE
    ```    
    
    You should see 12 ELA in the DID Sidechain wallet pre-loaded:
    ```
    {"Result":"12","Error":0,"Desc":"Success"}
    ```
 
7. Verify that your token sidechain is running correctly [Elastos.ELA.Sidechain.Token](http://github.com/elastos/Elastos.ELA.Sidechain.Token)

    Because "HttpRestPort" of token sidechain is not available, currently, it's not connected to the wallet service API so you cannot send any tokens from mainchain to token sidechain. All you can do is check that token sidechain is running properly. When the API layer for token sidechain will be available, this documentation will be updated.
    ```
    curl -H "Content-Type:application/json" -H "Accept:application/json" --data '{"method":"getbestblockhash"}' http://localhost:20616
    ```

    Should return something like
    ```
    {
        "id": null,
        "jsonrpc": "2.0",
        "result": "86f796f68d7422de0b3b27822697d5c255e653de3fcebfc86bd36828f41ace20",
        "error": null
    }
    ```

8. Verify the Wallet Service is running by checking the miner reward wallet:

    ```
    curl http://localhost:8091/api/1/balance/EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE
    ```    
    
    You should see at least 825 ELA in the miner wallet:
    ```
    {"result":"835.35142952","status":200}
    ```
    
9. Verify the DID Service is running by checking the pre-loaded wallet:

    ```
    curl http://localhost:8092/api/1/balance/EJWT3HbQWXNZk9gDwvGJwXdvv87qkdRkhE
    ```    
    
    You should see 12 ELA in the DID Sidechain wallet pre-loaded:
    ```
    {"result":"12.0","status":200}
    ```
    
10. Verify that cross-chain mainchain to sidechain transfers work
    NOTE: Make sure to give at least 1-2 minutes(after docker container has started) before you call the following API and do not re-attempt the same command because it might fail otherwise.
    ```
    curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE","privateKey": "2e900f236671edfd39a31e65a938491df5fc9a53b6b16e8ea0d697fe2f0a3d52"}],"receiver": [{"address": "EJWT3HbQWXNZk9gDwvGJwXdvv87qkdRkhE","amount": "1"}]}' localhost:8091/api/1/cross/m2d/transfer
    ```
    
    You should see the transaction hash returned and a 200 status
    ```
    {"result":"57507ad2eb5513e79f90912d789559ce36a387facbf0f481f96b0bba27f52385","status":200}
    ```
    
    In a minute or two you can also see the new ELA on your receiver address, it should be greater than the initial 12
    ```
    curl http://localhost:20604/api/v1/asset/balances/EJWT3HbQWXNZk9gDwvGJwXdvv87qkdRkhE
    ```
    
11. Verify that the API Misc works [Elastos.ORG.Misc.API](https://github.com/elastos/Elastos.ORG.API.Misc)

    The service for mainchain is running on port 8093 and for DID sidechain is running on port 8094
    
    ```
    curl http://localhost:8093/api/1/ping
    ```
    
    Should return
    
    ```
    {"result":"pong 1.0.1","status":200}
    ```

    ```
    curl http://localhost:8094/api/1/ping
    ```
    
    Should return
    
    ```
    {"result":"pong 1.0.1","status":200}
    ```

12. Verify that your HIVE IPFS peers are working correctly [Elastos.NET.Hive.IPFS](http://github.com/elastos/Elastos.NET.Hive.IPFS)

    ```
    curl http://localhost:38080/version
    ```

    Should return
    ```
    Commit: 6a5e24032
    Client Version: go-ipfs/0.4.18/6a5e24032
    Protocol Version: ipfs/0.1.0
    ```

13. Verify that your HIVE Cluster is working correctly [Elastos.NET.Hive.Cluster](http://github.com/elastos/Elastos.NET.Hive.Cluster)

    This service is running on port 9094-9095. There is only one node for the cluster so there's no redundant setup as this is a test environment. 
    
    9094 exposes Cluster API endpoints.
    ```
    curl http://localhost:9094/id
    ```

    Should return something like
    ```
    {
        "id": "QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H",
        "addresses": [
            "/p2p-circuit/ipfs/QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H",
            "/ip4/127.0.0.1/tcp/9096/ipfs/QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H",
            "/ip4/172.19.0.3/tcp/9096/ipfs/QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H"
        ],
        "cluster_peers": [
            "QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H"
        ],
        "cluster_peers_addresses": [],
        "version": "0.8.0+git65033f01bb94e5d205e1ed0e80198f050cea212a",
        "commit": "",
        "rpc_protocol_version": "/hivecluster/0.8/rpc",
        "error": "",
        "ipfs": {
            "id": "QmQNhoWCQivT7sJSezu8PnNpjA4rjKRxWHa47tFmkW3mHj",
            "addresses": [
            "/ip4/127.0.0.1/tcp/4001/ipfs/QmQNhoWCQivT7sJSezu8PnNpjA4rjKRxWHa47tFmkW3mHj",
            "/ip4/172.19.0.2/tcp/4001/ipfs/QmQNhoWCQivT7sJSezu8PnNpjA4rjKRxWHa47tFmkW3mHj"
            ],
            "error": ""
        },
        "peername": "Elastos Hive Privnet"
    }
    ```

    And 9095 exposes Node API endpoints.
    ```
    curl http://localhost:9095/api/v0/pin/ls
    ```

    Should return an empty dictionary as there's nothing present yet
    ```
    {"Keys":{}}
    ```

## Creating a DID, and Storing/Retrieving Metadata

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

Then you can call `POST /api/1/setDidInfo` to store data to this DID. There are two private keys, the outer private key
is the private key of the wallet address that is paying for the store data transaction, you can use the ELA stored on the DID sidechain in `sidechain-preloaded.json`
for this.

We will use the pre-configured DID to store info. Refer to test/action_storeinfoon_didchain.json for what to pass in the body of the request to this API endpoint.

```
cat test/action_storeinfoon_didchain.json
```

Should return
```
{
  "privateKey": "acf6ee13a2256f60f597f55bdd17d0ee772014db7233b50543bebdd1faf9a0da",
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
{"result":"e2fe03663dafa8898d021ba4b80822cbd01279a9d1611cb81531d11407a326bd","status":200}
```

If you try to set the DID info before letting it be propagated to the block, you might get an error with something like 
```
{
  "result": "double spent UTXO inputs detected, transaction hash: 37393136626431306661653962363231663763396539626530636433653264653535306365313766393632656462663430633839383232383061373761376562, input: cf00f4c5600a5d7ec4f89197a555ba1334e506d3ce4f03b7f98b283765693696, index: 0",
  "status": 10001
}
```

Don't be alarmed. Just wait a couple of minutes and try again.

### Retrieving the DID info must be on the Misc.API - port 8093

This is currently undocumented in the DID service docs because it's broken, so you need to use Misc.API which should be running on port `8093`.

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

NOTE: This API is unstable at the moment so things might not work as expected sometimes. If you encounter any issues, please submit a github issue and we'll look into it.
    

## Elastos Hive - IPFS Storage

Let's try to create a simple file and then push it to Elastos Hive using the API endpoints exposed via our Cluster setup.

1. Add the content "This is Elastos" to the cluster. Note that doing this will automatically pin this file to the cluster too.
    ```
    curl -F file="Hello, World" "http://localhost:9095/api/v0/file/add"
    ```

    Should return something like
    ```
    {
        "Name": "QmTev1ZgJkHgFYiCX7MgELEDJuMygPNGcinqBa2RmfnGFu",
        "Hash": "QmTev1ZgJkHgFYiCX7MgELEDJuMygPNGcinqBa2RmfnGFu",
        "Size": "20"
    }
    ```

2. Let's verify that something was indeed pinned to the cluster
    ```
    curl "http://localhost:9095/api/v0/pin/ls"
    ```

    Should return something like
    ```
    {"Keys":{"QmTev1ZgJkHgFYiCX7MgELEDJuMygPNGcinqBa2RmfnGFu":{"Type":"recursive"}}}
    ```

3. Retrieve the content from the hash using the cluster-slave
    ```
    curl http://localhost:9095/api/v0/file/cat?arg=QmTev1ZgJkHgFYiCX7MgELEDJuMygPNGcinqBa2RmfnGFu
    ```

    Should return
    ```
    Hello, World
    ```

4. Let's push some random content to the cluster by directly interacting with the IPFS peer nodes
    ```
    echo "This is Elastos" > hello.txt
    docker cp hello.txt ela-hive-ipfs-peer:/tmp/hello.txt
    docker exec ela-hive-ipfs-peer ipfs add /tmp/hello.txt
    ```

    Should return something like
    ```
    16 B / 16 B  100.00%added QmZVtqcb9AAz4xSXkWupWgA9mHDLwtksuem2fhNbNkYwbA hello.txt
    ```  

5. Check that this content was added to the cluster by trying to read this hash
    ```
    docker exec ela-hive-ipfs-peer ipfs cat QmZVtqcb9AAz4xSXkWupWgA9mHDLwtksuem2fhNbNkYwbA
    ```

    Should return
    ```
    This is Elastos
    ```

6. Let's verify the above content we added is actually in the cluster by interacting with the CLUSTER APIs
    ```
    curl http://localhost:9095/api/v0/file/cat?arg=QmZVtqcb9AAz4xSXkWupWgA9mHDLwtksuem2fhNbNkYwbA
    ```

    Should return
    ```
    This is Elastos
    ```

7. If you would like to read up on how to interact with the cluster and node APIs of elastos, please refer to [https://github.com/elastos/Elastos.NET.Hive.DevDocs](https://github.com/elastos/Elastos.NET.Hive.DevDocs)

## Smart Contracts - Ethereum Sidechain

This is coming soon and we won't declare this private net or the developer platform BETA ready until it is, 
since we recognize that smart contracts and storage are minimally required for functioning dApps
    

## More Documentation, `config.json` and other nuances if you want to tinker

A lot more details about the configuration and actual running of the blockchain is located here: [https://www.cyberrepublic.org/experimental-docs](https://www.cyberrepublic.org/experimental-docs)
