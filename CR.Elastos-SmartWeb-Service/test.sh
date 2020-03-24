#!/bin/bash

# Run postgres server as a docker container
cd tools
./postgres_test.sh
cd ..

# Run pytest
source venv/bin/activate
pip install -r requirements.txt
py.test grpc_adenine/implementations/common_test.py


# Cleanup
docker container rm -f smartweb-postgres-test
deactivate

