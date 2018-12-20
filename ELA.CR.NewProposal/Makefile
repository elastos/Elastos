VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
BUILD =go build -ldflags "-X main.Version=$(VERSION) -X 'main.GoVersion=`go version`'" #-race

all:
	$(BUILD) -o ela config.go main.go
	$(BUILD) cli/main/ela-cli.go

cli:
	$(BUILD) cli/main/ela-cli.go

format:
	go fmt ./*

clean:
	rm -rf *.8 *.o *.out *.6

