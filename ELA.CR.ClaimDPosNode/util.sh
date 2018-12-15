#!/bin/sh
if [ "$1" = "clean" ]; then
	find . -name "config.json" | sed 's#^#rm -fr #g' | sh
	find . -name "Chain_UnitTest" | sed 's#^#rm -fr #g' | sh
	find . -name "Chain_WhiteBox" | sed 's#^#rm -fr #g' | sh
	find . -name "Dpos_UnitTest" | sed 's#^#rm -fr #g' | sh
	find . -name "Logs" | sed 's#^#rm -fr #g' | sh
	find . -name "ArbiterLogs" | sed 's#^#rm -fr #g' | sh
	find . -name "DposEvent" | sed 's#^#rm -fr #g' | sh
elif [ "$1" = "test" ]; then
	./ela-cli script -f test/white_box/main/test_all.lua
fi

