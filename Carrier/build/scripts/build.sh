#!/bin/sh

# argv1: BUILD 

getOptFlags() {
    case $1 in
    "debug")
        _CFLAGS="-g3 -O0"
        ;;
    "release")
        _CFLAGS="-O2 -DNDEBUG"
        ;;
    *)
        _CFLAGS=
        ;;
    esac

    echo ${_CFLAGS} 
}

if [ x"$1" = x"configFlags" ]; then
    getOptFlags $2
    exit 0
fi

# argv1: 
conductOpt() {
    echo "TODO"
}

if [ x"$1" = x"conductOpt" ]; then
    conductOpt
    exit 0
fi

exit 0

