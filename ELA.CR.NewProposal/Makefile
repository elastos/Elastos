GOVER := $(shell go version)
VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
BUILD = go build -ldflags "-X main.Version=$(VERSION) -X 'main.GoVersion=$(GOVER)'" #-race

DEV_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
DEV_VERSION := $(shell git rev-list HEAD -n 1 | cut -c 1-8)
DEV_BUILD = go build -ldflags "-X main.Version=$(DEV_BRANCH)-$(DEV_VERSION) -X 'main.GoVersion=$(GOVER)'" #-race

all:
	$(BUILD) -o ela log.go  main.go
	$(BUILD) -o ela-cli cmd/ela-cli.go
	$(BUILD) -o ela-dns elanet/dns/main/main.go

dev:
	$(DEV_BUILD) -o ela log.go  main.go
	$(DEV_BUILD) -o ela-cli cmd/ela-cli.go
	$(BUILD) -o ela-dns elanet/dns/main/main.go

linux:
	GOARCH=amd64 GOOS=linux $(BUILD) -o ela log.go  main.go
	GOARCH=amd64 GOOS=linux $(BUILD) -o ela-cli cmd/ela-cli.go
	GOARCH=amd64 GOOS=linux $(BUILD) -o ela-dns elanet/dns/main/main.go

cli:
	$(BUILD) -o ela-cli cmd/ela-cli.go

dns:
	$(BUILD) -o ela-dns elanet/dns/main/main.go

tools:
	$(BUILD) -o ela-datagen benchmark/tools/generator/main.go
	$(BUILD) -o ela-inputcounter benchmark/tools/inputcounter/main.go

format:
	go fmt ./*

clean:
	rm -rf *.8 *.o *.out *.6

