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
    python3 -m grpc_tools.protoc --proto_path=grpc_adenine/definitions --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/common.proto;
    python3 -m grpc_tools.protoc --proto_path=grpc_adenine/definitions --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/hive.proto;
    python3 -m grpc_tools.protoc --proto_path=grpc_adenine/definitions --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/wallet.proto;
    python3 -m grpc_tools.protoc --proto_path=grpc_adenine/definitions --python_out=grpc_adenine/stubs/python --grpc_python_out=grpc_adenine/stubs/python grpc_adenine/definitions/sidechain_eth.proto;
    ```
- For generating golang client code
    ```
    go get -u github.com/golang/protobuf/protoc-gen-go;
    export PATH=$PATH:$GOPATH/bin;
    protoc -I=grpc_adenine/definitions --go_out=plugins=grpc:grpc_adenine/stubs/go grpc_adenine/definitions/health_check.proto;
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
