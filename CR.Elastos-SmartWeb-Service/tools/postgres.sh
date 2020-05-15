#!/bin/bash

docker container stop smartweb-postgres || true && docker container rm -f smartweb-postgres || true

# start a postgres docker container
docker run -d --name smartweb-postgres \
    -v "$PWD/.postgres-data:/var/lib/postgresql/data"     \
    -e POSTGRES_DB=smartweb                             \
    -e POSTGRES_USER=gmu                                \
    -e POSTGRES_PASSWORD=gmu                            \
    -p 5434:5432                                        \
    postgres:11-alpine
    
# wait for database to start
sleep 7
# Copy .sql files to the running container
docker cp ../grpc_adenine/database/scripts/create_table_scripts.sql smartweb-postgres:/create_table_scripts.sql
docker cp ../grpc_adenine/database/scripts/reset_database.sql smartweb-postgres:/reset_database.sql

# Run the sql scripts
reset=${1-no}
if [ "$reset" == "yes" ]
then
  echo "Resetting database"
  docker container exec -it smartweb-postgres psql -h localhost -d smartweb -U gmu -a -q -f /reset_database.sql
fi
docker container logs smartweb-postgres
docker container exec -it smartweb-postgres psql -h localhost -d smartweb -U gmu -a -q -f /create_table_scripts.sql



