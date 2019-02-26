#!/bin/sh

START=`date +%s`

print_progress () {
  printf "\e[0;33m$1\e[0m\n" >> ~/deploy.log
}

print_success () {
  printf "\e[4;32m$1\e[0m\n" >> ~/deploy.log
}

echo "Starting Elastos.ELA.Arbiter.1" > ~/deploy.log

cd /go/src/github.com/elastos/Elastos.ELA.Arbiter

nohup ./arbiter -p elastos > log.out 2>&1 &
print_progress "\nELA Arbiter 1 Started"

END=`date +%s`

print_success "\nDone. Runtime: $((END-START)) seconds."