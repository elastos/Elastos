# Elastos Local PrivateNet - v0.1

## This is official but unsupported and ALPHA version

These executables are built from the private repo [https://github.com/ClarenceL/ela-privnet-staging](https://github.com/ClarenceL/ela-privnet-staging) 
which we use to compile the executables into the right environment, only the executables can be found here.

#### These executables are purpose built for the docker images herein, if you want to use them directly you may need to build from source

If you are a core developer and need to update the executable please contact me for access.

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

## More Documentation, `config.json` and other nuances if you want to tinker

A lot more details about the configuration and actual running of the blockchain is located here: [https://www.cyberrepublic.org/experimental-docs](https://www.cyberrepublic.org/experimental-docs)
