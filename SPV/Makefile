BUILD=go build
VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)

BUILD_SPV_CLI =$(BUILD) -ldflags "-X main.Version=$(VERSION)" -o ela-wallet client.go
BUILD_SPV_SERVICE =$(BUILD) -ldflags "-X main.Version=$(VERSION)" -o service log.go main.go

all:
	$(BUILD_SPV_CLI)
	$(BUILD_SPV_SERVICE)