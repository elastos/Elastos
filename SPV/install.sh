#!/bin/bash

# Create golang.org/x/ folder
cd ../
mkdir golang.org/
mkdir golang.org/x/
cd golang.org/x/

git clone https://github.com/golang/crypto.git
git clone https://github.com/golang/net.git
git clone https://github.com/golang/sys.git

cd ../../SPVWallet/

go get github.com/itchyny/base58-go
go get github.com/howeyc/gopass
go get github.com/urfave/cli

cd ../github.com/
mkdir mattn
cd mattn
git clone https://github.com/mattn/go-sqlite3.git
cd ../../SPVWallet/
go install github.com/mattn/go-sqlite3
