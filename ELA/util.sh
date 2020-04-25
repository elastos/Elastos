#!/bin/bash

function cleanDirectoryRecursively() {
  find . -name "$1" | sed 's#^#rm -fr #g' | sh
}

function cleanTempData() {
  cleanDirectoryRecursively "config.json"
  cleanDirectoryRecursively "peers.json"
  cleanDirectoryRecursively "elastos_test"
}

cpuArgs=''
if [[ "$2" == "cpu" || "$3" == "cpu" ]]; then
  cpuArgs='-cpuprofile profile.out'
fi
memArgs=''
if [[ "$2" == "mem" || "$3" == "mem" ]]; then
  memArgs='-benchmem -memprofile memprofile.out'
fi

if [[ "$1" == "clean" ]]; then
  cleanTempData
elif [[ "$1" == "cleanall" ]]; then
  cleanTempData
  cleanDirectoryRecursively "elastos"
elif [[ "$1" == "test" ]]; then
  ./ela-cli script -f test/white_box/main/test_all.lua
elif [[ "$1" == "unittest" ]]; then
  go test `go list ./... | grep -v benchmark` -short
elif [[ "$1" == "benchall" ]]; then
  go test ./benchmark/... -bench=. $cpuArgs $memArgs
elif [[ "$1" == "benchspec" ]]; then
  go test ./benchmark/special/... -bench=. $cpuArgs $memArgs
elif [[ "$1" == "benchproc" ]]; then
  go test ./benchmark/process/... -bench=. $cpuArgs $memArgs
elif [[ "$1" == "benchsync" ]]; then
  cp ela benchmark/sync/
  cp config.json benchmark/sync/
  rm -rf benchmark/sync/elastos_test
  cp -r elastos benchmark/sync/elastos_test
  ulimit -n 1024
  go test ./benchmark/sync/... -bench=. $cpuArgs $memArgs -timeout 1000m
elif [[ "$1" == "datagen" ]]; then
  ./ela-datagen --dir benchmark/process/elastos_test --height "$2" --mode normal --inputsperblock 200 --maxrefers 200 --minrefers 100 --addresscount 100
fi
