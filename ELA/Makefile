VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
BUILD = go build -ldflags "-X main.Version=$(VERSION) -X 'main.GoVersion=`go version`'" #-race

all:
	$(BUILD) -o ela log.go config.go main.go
	$(BUILD) -o ela-cli cmd/ela-cli.go

cli:
	$(BUILD) -o ela-cli cmd/ela-cli.go

dns:
	$(BUILD) -o ela-dns elanet/dns/main/main.go

format:
	go fmt ./*

clean:
	rm -rf *.8 *.o *.out *.6

