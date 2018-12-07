VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
Minversion := $(shell date)
BUILD_ELA_PAR = -ldflags "-X github.com/elastos/Elastos.ELA/config.Version=$(VERSION)" #-race
BUILD_ELACLI_PAR = -ldflags "-X main.Version=$(VERSION)"

all:
	go build $(BUILD_ELA_PAR) -o ela main.go
	go build $(BUILD_ELACLI_PAR) ela-cli.go

client:
	go build $(BUILD_ELACLI_PAR) ela-cli.go

format:
	go fmt ./*

clean:
	rm -rf *.8 *.o *.out *.6
