# GRPC_python
GRPC python implementation with SQL Alchemy

## Run smartweb-service quickly(runs using docker)
Copy environment file and change variables as needed:
``` 
cp .env.example .env
```
``` 
./run.sh
```

## Run without using docker
#### Prerequisites

First, install Python3:

```
brew install python3 // On Mac
sudo apt-get install python3 // On Ubuntu
```

Normally, pip comes with python3 if you're downloading the latest version (or any version above 3.4). If that is not the case, install pip by running the following:

```
curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
python3 get-pip.py
```

Install virtualenv:
```
pip3 install virtualenv
```

Install solidity compiler:
- On Mac:
    ```
    brew update
    brew upgrade
    brew tap ethereum/ethereum
    brew install solidity
    ```
- On Ubuntu:
    ``` 
    sudo add-apt-repository ppa:ethereum/ethereum
    sudo apt-get update
    sudo apt-get install solc
    ```

#### Instructions on running
Clone the repository
```
git clone https://github.com/cyber-republic/elastos-smartweb-service.git
cd elastos-smartweb-service
```

To get the API service running, run the following terminal commands:
```
virtualenv -p `which python3` venv
```
```
source venv/bin/activate
```
```
pip3 install -r requirements.txt
```

Export Path:
```
export PYTHONPATH="$PYTHONPATH:$PWD/grpc_adenine/stubs/"
```

Copy environment file and change variables as needed:
``` 
# Change DB_HOST from '172.17.0.1' to 'localhost'
cp .env.example .env
```

Start Postgres server:
```
cd tools
# This script automatically runs the scripts located at grpc_adenine/database/scripts/
./postgres.sh
cd ..
```

Run the following command:
```
python3 grpc_adenine/server.py
```

## Additional Info:
Command to build protocol buffer files:
- For generating python client code
    ```
    python3 -m grpc_tools.protoc --proto_path=grpc_adenine/definitions --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/health_check.proto;
    python3 -m grpc_tools.protoc --proto_path=grpc_adenine/definitions --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/node_rpc.proto;
    python3 -m grpc_tools.protoc --proto_path=grpc_adenine/definitions --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/common.proto;
    python3 -m grpc_tools.protoc --proto_path=grpc_adenine/definitions --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/hive.proto;
    python3 -m grpc_tools.protoc --proto_path=grpc_adenine/definitions --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/wallet.proto;
    python3 -m grpc_tools.protoc --proto_path=grpc_adenine/definitions --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/sidechain_eth.proto;
    ```
- For generating golang client code
    ```
    # Install protocol compiler: https://github.com/protocolbuffers/protobuf/blob/master/README.md#protocol-compiler-installation
    # Install protoc-gen-go
    go get -u github.com/golang/protobuf/protoc-gen-go;
    export PATH=$PATH:$GOPATH/bin;
    protoc -I=grpc_adenine/definitions --go_out=plugins=grpc:grpc_adenine/stubs/go grpc_adenine/definitions/health_check.proto;
    protoc -I=grpc_adenine/definitions --go_out=plugins=grpc:grpc_adenine/stubs/go grpc_adenine/definitions/node_rpc.proto;
    protoc -I=grpc_adenine/definitions --go_out=plugins=grpc:grpc_adenine/stubs/go grpc_adenine/definitions/common.proto;
    protoc -I=grpc_adenine/definitions --go_out=plugins=grpc:grpc_adenine/stubs/go grpc_adenine/definitions/hive.proto;
    protoc -I=grpc_adenine/definitions --go_out=plugins=grpc:grpc_adenine/stubs/go grpc_adenine/definitions/wallet.proto;
    protoc -I=grpc_adenine/definitions --go_out=plugins=grpc:grpc_adenine/stubs/go grpc_adenine/definitions/sidechain_eth.proto;
    ```

## Debugging and Development Tools:
Build docker image:
```
docker build -t cyberrepublic/elastos-smartweb-service .
```
Push docker image:
```
docker push cyberrepublic/elastos-smartweb-service:latest
```
Connect to postgresql database
```
docker container exec -it smartweb-postgres psql -h localhost -U gmu -d smartweb_master
```
Look at the tables
```
\dt 
```
Get all items from the table 'users'
``` 
select * from users;
```

## Unit Testing

``` 
./test.sh
```

## Deploy to production(Uses Google Cloud Run)

https://cloud.google.com/endpoints/docs/grpc/get-started-cloud-run

```
# Only for the first time
gcloud run deploy elastos-smartweb-service-esp --image="gcr.io/endpoints-release/endpoints-runtime-serverless:2" --allow-unauthenticated  --platform managed --project=careful-pillar-269322;

# Every time there is an update
python3 -m grpc_tools.protoc --include_imports --include_source_info --proto_path=grpc_adenine/definitions --descriptor_set_out=api_descriptor.pb --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/*.proto;
# Replace all the subts with "from . import __"

# Build and push docker image
docker build -t cyberrepublic/elastos-smartweb-service .;
docker tag cyberrepublic/elastos-smartweb-service gcr.io/careful-pillar-269322/elastos-smartweb-service:latest;
docker push gcr.io/careful-pillar-269322/elastos-smartweb-service:latest;

# Deploy Endpoints config
gcloud endpoints services deploy api_descriptor.pb api_config.yaml;

# Build and run a new esp image
./gcloud_build_image.sh -s elastos-smartweb-service-esp-jgdewju65a-uk.a.run.app -c 2020-02-26r2 -p careful-pillar-269322;
gcloud run deploy elastos-smartweb-service-esp \
  --image="gcr.io/careful-pillar-269322/endpoints-runtime-serverless:elastos-smartweb-service-esp-jgdewju65a-uk.a.run.app-2020-02-26r2" \
  --set-env-vars=ESPv2_ARGS=--cors_preset=basic \
  --allow-unauthenticated \
  --platform managed \
  --project=careful-pillar-269322;
gcloud run services add-iam-policy-binding elastos-smartweb-service \
  --member "serviceAccount:268296012146-compute@developer.gserviceaccount.com" \
  --role "roles/run.invoker" \
  --platform managed \
  --project careful-pillar-269322
```