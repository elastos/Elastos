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
    
2. Check your ports, here are the following ports these containers expect available:
    - Mainchain: 20333-20338
    - DID Sidechain: 20604-20608
    - Wallet Service: 8091 - [https://walletservice.readthedocs.io](https://walletservice.readthedocs.io)
    - DID Service: 8092 - [https://didservice.readthedocs.io](https://didservice.readthedocs.io)
    - Add-on API: 8093 - [https://github.com/elastos/Elastos.ORG.API.Misc](https://github.com/elastos/Elastos.ORG.API.Misc)
    
    ##### Note: DID Service's - `getDidInfo` method is currently broken, but available on the Add-on API - port 8093
    
3. Just run with docker-compose from within the directory:
    
    ```
    docker-compose up --remove-orphans --build --force-recreate -d
    ```
    For users in China, if you get issues pulling images please refer to this post: https://segmentfault.com/a/1190000016083023
    
4. Verify the Mainchain is running by checking the miner reward wallet:

    ```
    http://localhost:20334/api/v1/asset/balances/EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE
    ```    
    
    You should see at least 825 ELA in the miner wallet:
    ```
    {"Desc":"Success","Error":0,"Result":"838.86741124"}
    ```
    
5. Verify the DID Sidechain is running by checking the pre-loaded wallet:

    ```
    http://localhost:20604/api/v1/asset/balances/EJWT3HbQWXNZk9gDwvGJwXdvv87qkdRkhE
    ```    
    
    You should see 12 ELA in the DID Sidechain wallet pre-loaded:
    ```
    {"Result":"12","Error":0,"Desc":"Success"}
    ```

6. Verify the Wallet Service is running by checking the miner reward wallet:

    ```
    http://localhost:8091/api/1/balance/EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE
    ```    
    
    You should see at least 825 ELA in the miner wallet:
    ```
    {"result":"835.35142952","status":200}
    ```
    
7. Verify the DID Service is running by checking the pre-loaded wallet:

    ```
    http://localhost:8092/api/1/balance/EJWT3HbQWXNZk9gDwvGJwXdvv87qkdRkhE
    ```    
    
    You should see 12 ELA in the DID Sidechain wallet pre-loaded:
    ```
    {"result":"12.0","status":200}
    ```
    
8. Verify that cross-chain mainchain to sidechain transfers work
    ```
    curl -X POST -H "Content-Type: application/json" -d '{"sender": [{"address": "EZngB4JXYAVhj8XZXR1HNWh2NkV5ttJtrE","privateKey": "2e900f236671edfd39a31e65a938491df5fc9a53b6b16e8ea0d697fe2f0a3d52"}],"receiver": [{"address": "EJWT3HbQWXNZk9gDwvGJwXdvv87qkdRkhE","amount": "1"}]}' localhost:8091/api/1/cross/m2d/transfer
    ```
    
    You should see the transaction hash returned and a 200 status
    ```
    {"result":"57507ad2eb5513e79f90912d789559ce36a387facbf0f481f96b0bba27f52385","status":200}
    ```
    
    In a minute or two you can also see the new ELA on your receiver address, it should be greater than the initial 12
    ```
    http://localhost:20604/api/v1/asset/balances/EJWT3HbQWXNZk9gDwvGJwXdvv87qkdRkhE
    ```
    
9. Verify that the API Misc works [Elastos.ORG.Misc.API](https://github.com/elastos/Elastos.ORG.API.Misc)

    This service is running on port 8093
    
    ```
    http://localhost:8093/api/1/ping
    ```
    
    Should return
    
    ```
    {"result":"pong 1.0.1","status":200}
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

Coming soon    


## Smart Contracts - Ethereum Sidechain

This is coming soon and we won't declare this private net or the developer platform BETA ready until it is, 
since we recognize that smart contracts and storage are minimally required for functioning dApps
    

## More Documentation, `config.json` and other nuances if you want to tinker

A lot more details about the configuration and actual running of the blockchain is located here: [https://www.cyberrepublic.org/experimental-docs](https://www.cyberrepublic.org/experimental-docs)
