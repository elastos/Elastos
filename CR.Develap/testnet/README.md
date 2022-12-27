## Services
- Main chain node: 21333-21339
- DID sidechain node: 21604-21608
- Token sidechain node: 21614-21618
- Ethereum sidechain node: 21634-21638
- API Misc Service for Main chain node: 21801
- API Misc Service for DID sidechain node: 21802
- MYSQL service: 21901

## Environment file
- .env

## How to connect to testnet
- `./prepare_docker.sh; docker-compose up --remove-orphans --build --force-recreate -d`

## How to interact with testnet
- Get current height of Main chain
```
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getcurrentheight"}' localhost:21336
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
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getcurrentheight"}' localhost:21606
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
curl -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"method":"getcurrentheight"}' localhost:21616
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
curl -X POST -H 'Content-Type: application/json' -H 'Accept:application/json' --data '{"jsonrpc":"2.0","method":"eth_blockNumber","params":[],"id":1}' localhost:21636
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