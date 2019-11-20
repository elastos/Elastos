# GRPC_python
GRPC python implementation with SQL Alchemy


## Prerequisites
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

## Instructions
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
```
cp .env.example .env
```

## Setting up PostgresSQL:

* Install PostgreSQL server 11.5

* Create a database by name 'smartweb_master'

* To start the PostgreSQL Server

```
pg_ctl -D /usr/local/var/postgres start
```

* Execute the create table scripts at smartweb_service/grpc_adenine/database/scripts/create_table_scripts.sql

* Execute the insert scripts at smartweb_service/grpc_adenine/database/scripts/insert_scripts.sql


## Start the server:

Export Path:
```
export PYTHONPATH="$PYTHONPATH:$PWD/grpc_adenine/stubs/"
```

Run the following command:
```
python3 grpc_adenine/server.py
```

### Additional Info:
Command to build protocol buffer files:
```
cd $PWD/grpc_adenine/
python3 -m grpc_tools.protoc -I definitions --python_out=stubs --grpc_python_out=stubs definitions/common.proto
```

Add a sample shared key to environment variable on the server:
```
export SHARED_SECRET_ADENINE="kHDP9V4JJbwd5GN"
```

