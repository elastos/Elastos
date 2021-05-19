#!/bin/sh

START=`date +%s`

print_progress () {
  printf "\e[0;33m$1\e[0m\n" | tee ~/deploy.log
}

print_success () {
  printf "\e[4;32m$1\e[0m\n" | tee ~/deploy.log
}

echo "Starting Elastos.ORG.Misc.API" | tee ~/deploy.log

cd /home/elauser

# wait until MySQL is really available
maxcounter=30

counter=1
while ! mysql -h mainnet-mysql -P 3306 -u root -p$MYSQL_ROOT_PASSWORD -e "show databases;" > /dev/null; do
    sleep 2
    echo "Waiting MySQL..." | tee ~/deploy.log
    counter=`expr $counter + 1`
    if [ $counter -gt $maxcounter ]; then
        echo "We have been waiting for MySQL too long already; failing." | tee ~/deploy.log
        exit 1
    fi;
done

# Run the SQL
mysql -h mainnet-mysql -P 3306 -u root -p$MYSQL_ROOT_PASSWORD -e "CREATE DATABASE IF NOT EXISTS chain_did"
mysql -h mainnet-mysql -P 3306 -u root -p$MYSQL_ROOT_PASSWORD -e "GRANT ALL PRIVILEGES ON chain_did.* TO '${MYSQL_USER}'@'%'"
mysql -h mainnet-mysql -P 3306 -u root -p$MYSQL_ROOT_PASSWORD -D chain_did < ./sql/global_setting.sql
mysql -h mainnet-mysql -P 3306 -u ${MYSQL_USER} -p$MYSQL_PASSWORD -D chain_did < ./sql/chain_chain_block_header.sql
mysql -h mainnet-mysql -P 3306 -u ${MYSQL_USER} -p$MYSQL_PASSWORD -D chain_did < ./sql/chain_chain_block_transaction_history.sql
mysql -h mainnet-mysql -P 3306 -u ${MYSQL_USER} -p$MYSQL_PASSWORD -D chain_did < ./sql/chain_chain_did_property.sql
mysql -h mainnet-mysql -P 3306 -u ${MYSQL_USER} -p$MYSQL_PASSWORD -D chain_did < ./sql/chain_chain_cmc_price.sql
mysql -h mainnet-mysql -P 3306 -u ${MYSQL_USER} -p$MYSQL_PASSWORD -D chain_did < ./sql/chain_chain_vote_info.sql
mysql -h mainnet-mysql -P 3306 -u ${MYSQL_USER} -p$MYSQL_PASSWORD -D chain_did < ./sql/chain_chain_producer_info.sql

print_progress "\nELA API Misc Sidechain" | tee ~/deploy.log

END=`date +%s`

print_success "\nDone. Runtime: $((END-START)) seconds." | tee ~/deploy.log

# Start it
./misc 2>&1