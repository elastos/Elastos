#!/bin/bash

# Run postgres server as a docker container
cd tools
./postgres.sh
cd ..

# Remove previous docker container:
docker container stop smartweb-service || true && docker container rm -f smartweb-service || true

# Run docker container
docker run --name smartweb-service \
  -v "$PWD/.env:/elastos-smartweb-service/.env" \
  -p 8000:8000                                  \
  cyberrepublic/elastos-smartweb-service:latest