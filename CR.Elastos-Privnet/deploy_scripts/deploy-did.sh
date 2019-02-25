#!/bin/sh

START=`date +%s`

print_progress () {
  printf "\e[0;33m$1\e[0m\n" >> ~/deploy.log
}

print_success () {
  printf "\e[4;32m$1\e[0m\n" >> ~/deploy.log
}

echo "Starting Elastos.ELA.SideChain.ID" > ~/deploy.log

cd /go/src/github.com/elastos/Elastos.ELA.SideChain.ID

nohup ./did > log.out 2>&1 &
print_progress "\nELA DID SideChain Started"

END=`date +%s`

print_success "\nDone. Runtime: $((END-START)) seconds."