#!/bin/bash

function cleanDirectoryRecursively() {
	find . -name $1 | sed 's#^#rm -fr #g' | sh
}

function cleanTempData() {
	cleanDirectoryRecursively "config.json"
	cleanDirectoryRecursively "peers.json"
	cleanDirectoryRecursively "elastos_test"
}

if [[ "$1" = "clean" ]]; then
    cleanTempData
elif [[ "$1" = "cleanall" ]]; then
    cleanTempData
	cleanDirectoryRecursively "elastos"
elif [[ "$1" = "test" ]]; then
	./ela-cli script -f test/white_box/main/test_all.lua
fi
