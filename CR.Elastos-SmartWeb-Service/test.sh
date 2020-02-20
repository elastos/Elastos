#!/bin/bash

# Run postgres server as a docker container
cd tools
./postgres_test.sh
cd ..

# Run pytest
py.test grpc_adenine/implementations/common_test1.py

# Remove previous docker container:
docker container rm -f smartweb-postgres-test
