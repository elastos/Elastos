## General Info

- Prerequisite basic knowledge of docker is expected  
- For the docker images that might be used for connecting to mainnet, testnet, regnet or private net, check out [https://cloud.docker.com/u/cyberrepublic/repository/list](https://cloud.docker.com/u/cyberrepublic/repository/list)

## Tools 

- [build_dockerimages.sh](./tools/build_dockerimages.sh): This shell script automatically builds all the binaries for carrier nodes and then packages them to be run inside docker images and if the flags "-p" and "-l" are set to "yes", the built docker images are automatically pushed to [Cyber Republic Docker Hub](https://cloud.docker.com/u/cyberrepublic/repository/list). Note that you need permission to push to the CR dockerhub but you can still build the images locally if you so choose
- [staging_tools.md](./tools/staging_tools.md): This README file contains all the commands that are used in building the private net from scratch(if that's your cup of tea)

## Tests

## Repos used to build 

- [Elastos.NET.Carrier.Bootstrap](https://github.com/elastos/Elastos.NET.Carrier.Bootstrap): release-v5.2

## Containers that are run

### Bootstrap Nodes

- ela-carrier-bootstrap-1: 33445

## How to Run

1. Just run with docker-compose from within the corresponding directory:
    
    ```
    docker-compose up --remove-orphans --build --force-recreate -d
    ```
    For users in China, if you get issues pulling images please refer to this post: https://segmentfault.com/a/1190000016083023

    NOTE: If you run this, currently, the bootstrapd.conf is configured to point to production elastos carrier network. In the future, capability to connect to private net will be provided