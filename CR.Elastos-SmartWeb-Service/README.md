# GRPC_python
GRPC python implementation with SQL Alchemy


## Prerequisites
For macOS, to install python using homebrew:
```
$brew install python3
```

For Linux based OS, to install python:
```
$apt-get install python3
```

Install virtualenv:
```
$pip3 install virtualenv
```

## Instructions
Clone the repository
```
$git clone https://github.com/cyber-republic/elastos-smartweb-service.git
$cd elastos-smartweb-service
```

Export Path:
```
$export PYTHONPATH="$PYTHONPATH:/*your_path*/elastos-smartweb-service/grpc_adenine/stubs/"
```

To get the API service running, run the following terminal commands:
```
$virtualenv -p `which python3` venv
```
```
$source venv/bin/activate
```
```
(venv)$pip install -r requirements.txt
```

## Setting up PostgresSQL:

* Install PostgreSQL server 11.5

* Create a database by name 'smartweb_master'

* To start the PostgreSQL Server

```
$pg_ctl -D /usr/local/var/postgres start
```

* Execute the create table scripts at smartweb_service/grpc_adenine/database/scripts/create_table_scripts.sql

* Execute the insert scripts at smartweb_service/grpc_adenine/database/scripts/insert_scripts.sql


## Start the server:
```
(venv)$python grpc_adenine/server.py
```

### Additional Info:
Command to build protocol buffer files:
```
$python -m grpc_tools.protoc -I definitions --python_out=stubs --grpc_python_out=stubs definitions/common.proto
```

Add a sample shared key to environment variable on the server:
```
$export SHARED_SECRET_ADENINE="kHDP9V4JJbwd5GN"
```

