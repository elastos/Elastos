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
- `docker-compose up --remove-orphans --build --force-recreate -d`

## If you're a developer and want to tinker with the configs yourself:
- Modify .env file to suit your needs
- Modify all the appropriate config files wherever needed:
    - mainchain/config.json
    - sidechain/did/config.json
    - sidechain/token/config.json
    - restful-services/api-misc/mainchain/config.json
    - restful-services/api-misc/sidechain/did/config.json
- Execute the command: `docker-compose up --remove-orphans --build --force-recreate -d`