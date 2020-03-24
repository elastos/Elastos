#!/bin/bash

# Run postgres server as a docker container
cd tools
./postgres.sh
cd ..

# Build docker container
docker build -t cyberrepublic/elastos-smartweb-service .

# Remove previous docker container:
docker container stop elastos-smartweb-service || true && docker container rm -f elastos-smartweb-service || true

# Run docker container
docker run --name elastos-smartweb-service \
  -v "$PWD/.env:/elastos-smartweb-service/.env" \
  -v "$PWD/tools/server.crt:/elastos-smartweb-service/tools/server.crt" \
  -v "$PWD/tools/server.key:/elastos-smartweb-service/tools/server.key" \
  -p 8001:8001                                  \
  cyberrepublic/elastos-smartweb-service:latest