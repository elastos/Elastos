#!/bin/bash

# Run postgres server as a docker container
cd tools
./postgres_test.sh yes
cd ..

# Run pytest
source venv/bin/activate
pip install -q -r requirements.txt
py.test --disable-pytest-warnings -s grpc_adenine/implementations/common_test.py

# Cleanup
docker container rm -f smartweb-postgres-test
deactivate

