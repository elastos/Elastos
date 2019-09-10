## Services
- Main chain node: 20333-20339
- DID sidechain node: 20604-20608
- Token sidechain node: 20614-20618
- API Misc Service for Main chain node: 20801
- API Misc Service for DID sidechain node: 20802
- MYSQL service: 20901

## Environment file
- .env

## How to connect to mainnet
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