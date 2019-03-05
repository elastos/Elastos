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
- `did-sidechain-preloaded.json` - Pre-loaded transferred ELA from the mainchain to this DID sidechain wallet 

## How to Run

1. Download and install Docker - this has been tested on the versions:
    - Docker Engine 18.09.2
    - Docker Compose 1.23.2
    - Docker Machine 0.16.1

2. The following are the GIT SHAs we're using for each project:
    - Elastos.ELA: tag v0.2.2 3f1e5e6
    - Elastos.ELA.SideChain.ID: tag v0.0.2 07599ad
    - Elastos.ELA.Arbiter: tag v0.0.3 8970efe
    - Elastos.ORG.Wallet.Service: master 49dcbfa
    - Elastos.ORG.DID.Service: master 1ff8c00
    - Elastos.ORG.API.Misc: master 5322f2e
    - Elastos.NET.Hive.IPFS: dev-master 6a5e240
    - Elastos.NET.Hive.Cluster: dev-master 65033f0
    
3. Check your ports, here are the following ports these containers expect available:
    - Mainchain: 20333-20338
    - DID Sidechain: 20604-20608
    - Wallet Service: 8091 - [https://walletservice.readthedocs.io](https://walletservice.readthedocs.io)
    - DID Service: 8092 - [https://didservice.readthedocs.io](https://didservice.readthedocs.io)
    - Add-on API: 8093 - [https://github.com/elastos/Elastos.ORG.API.Misc](https://github.com/elastos/Elastos.ORG.API.Misc)
    - HIVE IPFS: 38080, 48080 - [http://github.com/elastos/Elastos.NET.Hive.IPFS](http://github.com/elastos/Elastos.NET.Hive.IPFS)
    - HIVE Cluster: 9094-9095, 49094-49095 - [http://github.com/elastos/Elastos.NET.Hive.Cluster](http://github.com/elastos/Elastos.NET.Hive.Cluster)
    
    ##### Note: DID Service's - `getDidInfo` method is currently broken, but available on the Add-on API - port 8093
    
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

7. Verify the Wallet Service is running by checking the miner reward wallet:

    ```
    curl http://localhost:8091/api/1/balance/EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE
    ```    
    
    You should see at least 825 ELA in the miner wallet:
    ```
    {"result":"835.35142952","status":200}
    ```
    
8. Verify the DID Service is running by checking the pre-loaded wallet:

    ```
    curl http://localhost:8092/api/1/balance/EJWT3HbQWXNZk9gDwvGJwXdvv87qkdRkhE
    ```    
    
    You should see 12 ELA in the DID Sidechain wallet pre-loaded:
    ```
    {"result":"12.0","status":200}
    ```
    
9. Verify that cross-chain mainchain to sidechain transfers work
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
    
10. Verify that the API Misc works [Elastos.ORG.Misc.API](https://github.com/elastos/Elastos.ORG.API.Misc)

    This service is running on port 8093
    
    ```
    curl http://localhost:8093/api/1/ping
    ```
    
    Should return
    
    ```
    {"result":"pong 1.0.1","status":200}
    ```
11. Verify that your HIVE IPFS peers are working correctly [Elastos.NET.Hive.IPFS](http://github.com/elastos/Elastos.NET.Hive.IPFS)

    This service is running on port 38080 and 48080 since there are two peer nodes running
    ```
    curl http://localhost:38080/version
    ```

    Should return
    ```
    Commit: 6a5e24032
    Client Version: go-ipfs/0.4.18/6a5e24032
    Protocol Version: ipfs/0.1.0
    ```

12. Verify that your HIVE Cluster is working correctly [Elastos.NET.Hive.Cluster](http://github.com/elastos/Elastos.NET.Hive.Cluster)

    This service is running on port 9094-9095 and 49094-49095 since there are two cluster nodes running. 9094 exposes Cluster API endpoints.
    ```
    curl http://localhost:9094/id
    ```

    Should return something like
    ```
    {"id":"QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H","addresses":["/ip4/172.19.0.4/tcp/9096/ipfs/QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H","/p2p-circuit/ipfs/QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H","/ip4/127.0.0.1/tcp/9096/ipfs/QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H"],"cluster_peers":["QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H","QmXgeSALT56nSjNTw4qLXsH5cqNffTCkGsz5TBM7gwLors"],"cluster_peers_addresses":["/ip4/127.0.0.1/tcp/9096/ipfs/QmXgeSALT56nSjNTw4qLXsH5cqNffTCkGsz5TBM7gwLors","/ip4/172.19.0.5/tcp/9096/ipfs/QmXgeSALT56nSjNTw4qLXsH5cqNffTCkGsz5TBM7gwLors","/p2p-circuit/ipfs/QmXgeSALT56nSjNTw4qLXsH5cqNffTCkGsz5TBM7gwLors"],"version":"0.8.0+git65033f01bb94e5d205e1ed0e80198f050cea212a","commit":"","rpc_protocol_version":"/hivecluster/0.8/rpc","error":"","ipfs":{"id":"QmQNhoWCQivT7sJSezu8PnNpjA4rjKRxWHa47tFmkW3mHj","addresses":["/ip4/127.0.0.1/tcp/4001/ipfs/QmQNhoWCQivT7sJSezu8PnNpjA4rjKRxWHa47tFmkW3mHj","/ip4/172.19.0.3/tcp/4001/ipfs/QmQNhoWCQivT7sJSezu8PnNpjA4rjKRxWHa47tFmkW3mHj"],"error":""},"peername":"Elastos Hive Privnet"}
    ```

    And 9095 exposes Node API endpoints.
    ```
    curl http://localhost:9095/api/v0/pin/ls
    ```

    Should return an empty dictionary as there's nothing present yet
    ```
    {"Keys":{}}
    ```

13. Verify that your token sidechain is running correctly[Elastos.ELA.Sidechain.Token](http://github.com/elastos/Elastos.ELA.Sidechain.Token)

    Because "HttpRestPort" of token sidechain is not available, currently, it's not connected to the wallet service API so you cannot send any tokens from mainchain to token sidechain. All you can do is check that token sidechain is running properly. When the API layer for token sidechain will be available, this documentation will be updated.
    ```
    curl -H "Content-Type:application/json" -H "Accept:application/json" --data '{"method":"getbestblockhash"}' http://localhost:20616
    ```

    Should return something like
    ```
    {"id":null,"jsonrpc":"2.0","result":"5aeabb55d9ecf590ec5d189c7d6ee795648b9e000892e9570649cea5e85dbd38","error":null}
    ```

## Creating a DID, and Storing/Retrieving Metadata

Generally you will use the DID Service running on port `8092` for this - [https://didservice.readthedocs.io](https://didservice.readthedocs.io).

See "Create DID" for how to create a DID, you will receive both a did and a private key, store this somewhere.

```
GET /api/1/gen/did HTTP/1.1
```

[https://didservice.readthedocs.io/en/latest/api_guide.html#create-did](https://didservice.readthedocs.io/en/latest/api_guide.html#create-did)

Then you can call `POST /api/1/setDidInfo` to store data to this DID. There are two private keys, the outer private key
is the private key of the wallet address that is paying for the store data transaction, you can use the ELA stored on the DID sidechain in `did-sidechain-preloaded.json`
for this.

```
{
  "privateKey":"C740869D015E674362B1F441E3EDBE1CBCF4FE8B709AA1A77E5CCA2C92BAF99D", 
  "settings":{
      "privateKey":"E763239857B390502289CF75FF06EEEDC3252A302C50E1CBB7E5FAC8A703486F",
      "info":{
                  "Tag":"DID Property",
                  "Ver":"1.0",
                  "Status":1,
                  "Properties": [
                      {
                          "Key":"clark",
                          "Value":"hello,world",
                          "Status": 1
                      }
                  ]
      }
  }
}
```

The inner settings struct is the actual DID to modify, so you will use the private key from `/api/1/gen/did` here to specify that DID.

There is a cost of 10,000 SELA per 1kb on this privatenet, actual cost for the mainnet is not finalized.   

### Retrieving the DID info must be on the Misc.API - port 8093

This is currently undocumented in the DID service docs because it's broken, so you need to use Misc.API which should be running on port `8093`.

The API call should be `http://localhost:8093/api/1/did/{did}/{key}`

For example if you stored the property key "clark" above, and assuming the did was `iWNFAVtCuyNSNqHbJRQ3PVKgokCWLyVYHe`, then calling

`http://localhost:8093/api/1/did/iWNFAVtCuyNSNqHbJRQ3PVKgokCWLyVYHe/clark`

Would return the value `"hello,world"`.
    

## Elastos Hive - IPFS Storage

Let's try to create a simple file and then push it to Elastos Hive using the API endpoints exposed via our Cluster setup.

1. Add the content "This is Elastos" to the cluster. Note that doing this will automatically pin this file to the cluster too.
    ```
    curl -F file="This is Elastos" "http://localhost:9095/api/v0/file/add"
    ```

    Should return something like
    ```
    {"Name":"QmSDL6wJhKsTNudiLJoTkMU2tq2siH2DYQcyFN2Qz7JURg","Hash":"QmSDL6wJhKsTNudiLJoTkMU2tq2siH2DYQcyFN2Qz7JURg","Size":"23"}
    ```

2. Let's verify that something was indeed pinned to the cluster
    ```
    curl "http://localhost:9095/api/v0/pin/ls"
    ```

    Should return something like
    ```
    {"Keys":{"QmSDL6wJhKsTNudiLJoTkMU2tq2siH2DYQcyFN2Qz7JURg":{"Type":"recursive"}}}
    ```

3. Retrieve the content from the hash using the cluster-slave
    ```
    curl http://localhost:49095/api/v0/file/cat?arg=QmSDL6wJhKsTNudiLJoTkMU2tq2siH2DYQcyFN2Qz7JURg
    ```

    Note that even if we added the content via port 9095(ela-hive-cluster-master), we're now using port 49095(ela-hive-cluster-slave) to retrieve the result because we know that they're part of the same cluster and the same network. The above should return
    ```
    This is Elastos
    ```

4. Let's push the content to the cluster by directly interacting with the IPFS peer nodes
    ```
    echo "This is Elastos" > hello.txt
    docker cp hello.txt ela-hive-ipfs-peer-1:/tmp/hello.txt
    docker exec ela-hive-ipfs-peer-1 ipfs add /tmp/hello.txt
    ```

    Should return something like
    ```
    16 B / 16 B  100.00%added QmZVtqcb9AAz4xSXkWupWgA9mHDLwtksuem2fhNbNkYwbA hello.txt
    ```  

5. Check that this content was added to the cluster by trying to read this hash
    ```
    docker exec ela-hive-ipfs-peer-2 ipfs cat QmZVtqcb9AAz4xSXkWupWgA9mHDLwtksuem2fhNbNkYwbA
    ```

    Note that even if I added file from ela-hive-ipfs-peer-1 container, I'm using ela-hive-ipfs-peer-2 to cat out the content from the cluster. This is because these two peer nodes are part of the same IPFS swarm. The above should return
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
