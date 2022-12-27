## Services
- Main chain node: 20333-20339
- DID sidechain node: 20604-20608
- Token sidechain node: 20614-20618
- Ethereum sidechain node: 20634-20638
- API Misc Service for Main chain node: 20801
- API Misc Service for DID sidechain node: 20802
- MYSQL service: 20901

## Environment file
- .env

## How to connect to testnet
- `docker-compose up --remove-orphans --build --force-recreate -d`

## How to interact with testnet
- Get current height of Main chain
```
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getcurrentheight"}' localhost:20336
```
Should return something like
```
{
  "error": null,
  "id": null,
  "jsonrpc": "2.0",
  "result": :162008
}
```
- Get current height of DID sidechain
```
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getcurrentheight"}' localhost:20606
```
Should return something like
```
{
  "id": null,
  "jsonrpc": "2.0",
  "result": 2070,
  "error": null
}
```
- Get current height of Token sidechain
```
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getcurrentheight"}' localhost:20616
```
Should return something like
```
{
  "id": null,
  "jsonrpc": "2.0",
  "result": 2179,
  "error": null
}
```
- Get current block count of Eth sidechain
```
curl -X POST -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"jsonrpc":"2.0","method":"eth_blockNumber","params":[],"id":1}' localhost:20636
```
Should return something like
```
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": "0x1b8b"
}
```

## If you're a developer and want to tinker with the configs yourself:
- Modify .env file to suit your needs
- Modify all the appropriate config files wherever needed:
    - mainchain/config.json
    - sidechain/did/config.json
    - sidechain/token/config.json
    - restful-services/api-misc/mainchain/config.json
    - restful-services/api-misc/sidechain/did/config.json
- Execute the command: `docker-compose up --remove-orphans --build --force-recreate -d`
- Location of blockchain data: "~/.data/docker/volumes/development-services/testnet"