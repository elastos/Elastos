#!/bin/bash

# Run postgres server as a docker container
cd tools
./postgres_.sh
cd ..

# Run pytest
py.test grpc_adenine/implementations/common_test.py

# Remove previous docker container:
docker container rm -f smartweb-postgres-test