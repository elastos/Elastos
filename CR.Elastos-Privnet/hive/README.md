## General Info

- Prerequisite basic knowledge of docker is expected  
- After starting, the hive network will automatically start running and about 2 containers are created

## Repos used to build 

- Elastos.NET.Hive.IPFS: dev-master 6a5e24032b0d0c79e06a103f1b4078a648fa0e2e
- Elastos.NET.Hive.Cluster: dev-master 390b912df5cf496b7c9e6e5d3f6e0fb897a76db1

## Containers that are run

### Peer Nodes

- Peer node 1: 9093

### Cluster Nodes

- Cluster node 1: 9094-9096

## How to Run

1. Just run with docker-compose from within the corresponding directory:
    
    ```
    docker-compose up --remove-orphans --build --force-recreate -d
    ```
    For users in China, if you get issues pulling images please refer to this post: https://segmentfault.com/a/1190000016083023

2. Verify that your HIVE IPFS peer is working correctly [Elastos.NET.Hive.IPFS](http://github.com/elastos/Elastos.NET.Hive.IPFS)

    ```
    curl http://localhost:9093/version
    ```

    Should return
    ```
    Commit: 6a5e24032
    Client Version: go-ipfs/0.4.18/6a5e24032
    Protocol Version: ipfs/0.1.0
    ```

3. Verify that your HIVE Cluster is working correctly [Elastos.NET.Hive.Cluster](http://github.com/elastos/Elastos.NET.Hive.Cluster)

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

## Hive Storage Testing

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

## Resources

If you would like to read up on how to interact with the cluster and node APIs of elastos, please refer to [https://github.com/elastos/Elastos.NET.Hive.DevDocs](https://github.com/elastos/Elastos.NET.Hive.DevDocs)