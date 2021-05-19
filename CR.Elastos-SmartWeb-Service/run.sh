#!/bin/bash

# Run postgres server as a docker container
cd tools
./postgres.sh
cd ..

virtualenv -p `which python3` venv

source venv/bin/activate

pip install -r requirements.txt

export PYTHONPATH="$PYTHONPATH:$PWD/grpc_adenine/stubs/"

python grpc_adenine/server.py