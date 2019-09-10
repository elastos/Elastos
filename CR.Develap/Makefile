# Version
VERSION=0.0.1

# Architecture
ARCH=amd64

# Basic go commands
GOCMD=go
GOBUILD=$(GOCMD) build
GOCLEAN=$(GOCMD) clean
LDFLAGS="-X main.VERSION=$(VERSION)"
BINARY_NAME=develap

all: build

build-all: build build-linux build-darwin build-windows

build: 
	$(GOBUILD) -o $(BINARY_NAME) -ldflags $(LDFLAGS) -v

clean: 
	@rm -rf ./develap-binaries/linux_$(ARCH)/develap
	@rm -rf ./develap-binaries/darwin_$(ARCH)/develap
	@rm -rf ./develap-binaries/windows_$(ARCH)/develap.exe
	$(GOCLEAN)
      
# Cross compilation
build-linux:
	@mkdir -p ./develap-binaries/linux_$(ARCH)
	GOOS=linux GOARCH=$(ARCH) $(GOBUILD) -o ./develap-binaries/linux_$(ARCH)/$(BINARY_NAME) -ldflags $(LDFLAGS) -v
build-darwin:
	@mkdir -p ./develap-binaries/darwin_$(ARCH)
	GOOS=darwin GOARCH=$(ARCH) $(GOBUILD) -o ./develap-binaries/darwin_$(ARCH)/$(BINARY_NAME) -ldflags $(LDFLAGS) -v
build-windows:
	@mkdir -p ./develap-binaries/windows_$(ARCH)
	GOOS=windows GOARCH=$(ARCH) $(GOBUILD) -o ./develap-binaries/windows_$(ARCH)/$(BINARY_NAME).exe -ldflags $(LDFLAGS) -v

version:
	@echo $(VERSION)