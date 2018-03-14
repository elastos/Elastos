BUILD=go build
VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)

BUILD_SPV_CLI =$(BUILD) -ldflags "-X main.Version=$(VERSION)" -o wallet-cli client.go
BUILD_SPV_SERVICE =$(BUILD) -ldflags "-X main.Version=$(VERSION)" -o spv-wallet wallet.go

all:
	$(BUILD_SPV_CLI)
	$(BUILD_SPV_SERVICE)

install:
	chmod 777 install.sh
	./install.sh
