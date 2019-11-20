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

## Starting Postgres server:
```````````
cd tools

./postgres.sh

pg_ctl -D /usr/local/var/postgres start

```````````
```````````
If you encounter an error

Try

postgres -D /usr/local/var/postgres
You might see

FATAL:  lock file "postmaster.pid" already exists
HINT:  Is another postmaster (PID 449) running in data directory "/usr/local/var/postgres"?
Then try

kill -9 PID
example

kill -9 419

`````````````

## Execute the following command to create and insert values into your database 

docker container exec -it nucleus-postgres psql -h localhost -U gmu -d smartweb_master

To create tables

* Execute the contents at the location smartweb_service/grpc_adenine/database/scripts/create_table_scripts.sql 

To insert values

* Execute the contents at the location smartweb_service/grpc_adenine/database/scripts/insert_scripts.sql
`````````````
quit

`````````````
cd 

`````````````
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
