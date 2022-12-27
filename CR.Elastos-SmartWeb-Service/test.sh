#!/bin/bash

# Run postgres server as a docker container
cd tools
./postgres_test.sh yes
cd ..

# Run pytest
virtualenv -p `which python3` venv
source venv/bin/activate
pip install -q -r requirements.txt
SHARED_SECRET_ADENINE=7XDnFBdHafpPyIC4nrtuJ5EUYVqdEKjW DB_NAME=smartweb_test DB_PORT=5436 py.test --disable-pytest-warnings -v -xs grpc_adenine/implementations/common_test.py
DB_NAME=smartweb_test DB_PORT=5436 py.test --disable-pytest-warnings -v -xs grpc_adenine/implementations/hive_test.py
DB_NAME=smartweb_test DB_PORT=5436 py.test --disable-pytest-warnings -v -xs grpc_adenine/implementations/wallet_test.py
DB_NAME=smartweb_test DB_PORT=5436 py.test --disable-pytest-warnings -v -xs grpc_adenine/implementations/sidechain_eth_test.py
DB_NAME=smartweb_test DB_PORT=5436 py.test --disable-pytest-warnings -v -xs grpc_adenine/implementations/node_rpc_test.py

# Cleanup
docker container rm -f smartweb-postgres-test
deactivate

