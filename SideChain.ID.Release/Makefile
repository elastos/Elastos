GOFMT=gofmt
GC=go build
VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)

BUILD_NODE_PAR = -ldflags "-X main.Version=$(VERSION) -X 'main.GoVersion=`go version`'" #-race

all:
	$(GC) $(BUILD_NODE_PAR) -o did config.go log.go main.go

linux:
	GOARCH=amd64 GOOS=linux $(GC) $(BUILD_NODE_PAR) -o did config.go log.go main.go

format:
	$(GOFMT) -w main.go

clean:
	rm -rf *.8 *.o *.out *.6
