#!/bin/sh

START=`date +%s`

print_progress () {
  printf "\e[0;33m$1\e[0m\n" >> ~/deploy.log
}

print_success () {
  printf "\e[4;32m$1\e[0m\n" >> ~/deploy.log
}

echo "Starting Elastos.ORG.Misc.API" >> ~/deploy.log

cd /home/elauser

# wait until MySQL is really available
maxcounter=30

counter=1
while ! mysql -h ela-mysql -P 3306 -u root --password="12345678" -e "show databases;" > /dev/null; do
    sleep 2
    echo "Waiting MySQL..." >> ~/deploy.log
    counter=`expr $counter + 1`
    if [ $counter -gt $maxcounter ]; then
        echo "We have been waiting for MySQL too long already; failing." >> ~/deploy.log
        exit 1
    fi;
done

# Run the SQL
mysql -h ela-mysql -P 3306 -u root --password="12345678" -e "CREATE DATABASE IF NOT EXISTS chain_did"
mysql -h ela-mysql -P 3306 -u root --password="12345678" -e "GRANT ALL PRIVILEGES ON chain_did.* TO 'elastos'@'%'"
mysql -h ela-mysql -P 3306 -u root --password="12345678" -D chain_did < ./sql-init/global_setting.sql
mysql -h ela-mysql -P 3306 -u elastos --password="12345678" -D chain_did < ./sql-init/chain_chain_block_header.sql
mysql -h ela-mysql -P 3306 -u elastos --password="12345678" -D chain_did < ./sql-init/chain_chain_block_transaction_history.sql
mysql -h ela-mysql -P 3306 -u elastos --password="12345678" -D chain_did < ./sql-init/chain_chain_did_property.sql

print_progress "\nELA API Misc TEST"

END=`date +%s`

print_success "\nDone. Runtime: $((END-START)) seconds."

# Start it
./elaChain 2>&1 &