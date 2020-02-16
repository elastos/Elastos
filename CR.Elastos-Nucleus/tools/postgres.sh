#!/bin/bash

docker container stop nucleus-postgres || true && docker container rm -f nucleus-postgres || true

# start a postgres docker container
docker run -d --name nucleus-postgres \
    -v "$PWD/.postgres-data:/var/lib/postgresql/data"     \
    -e POSTGRES_DB=nucleus                             \
    -e POSTGRES_USER=gmu                                \
    -e POSTGRES_PASSWORD=gmu                            \
    -p 5432:5432                                        \
    postgres:11-alpine