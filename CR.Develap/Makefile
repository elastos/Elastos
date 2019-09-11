# Version
VERSION=0.1.0

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
	@cp -r ./mainnet/mainchain ./mainnet/did ./mainnet/token ./develap-binaries/linux_$(ARCH)/mainnet/
	@cp -r ./testnet/mainchain ./testnet/did ./testnet/token ./develap-binaries/linux_$(ARCH)/testnet/
	@zip -r ./develap-binaries/linux_$(ARCH)-v$(VERSION).zip ./develap-binaries/linux_$(ARCH)
build-darwin:
	@mkdir -p ./develap-binaries/darwin_$(ARCH)
	GOOS=darwin GOARCH=$(ARCH) $(GOBUILD) -o ./develap-binaries/darwin_$(ARCH)/$(BINARY_NAME) -ldflags $(LDFLAGS) -v
	@cp -r ./mainnet/mainchain ./mainnet/did ./mainnet/token ./develap-binaries/darwin_$(ARCH)/mainnet/
	@cp -r ./testnet/mainchain ./testnet/did ./testnet/token ./develap-binaries/darwin_$(ARCH)/testnet/
	@zip -r ./develap-binaries/darwin_$(ARCH)-v$(VERSION).zip ./develap-binaries/darwin_$(ARCH)
build-windows:
	@mkdir -p ./develap-binaries/windows_$(ARCH)
	GOOS=windows GOARCH=$(ARCH) $(GOBUILD) -o ./develap-binaries/windows_$(ARCH)/$(BINARY_NAME).exe -ldflags $(LDFLAGS) -v
	@cp -r ./mainnet/mainchain ./mainnet/did ./mainnet/token ./develap-binaries/windows_$(ARCH)/mainnet/
	@cp -r ./testnet/mainchain ./testnet/did ./testnet/token ./develap-binaries/windows_$(ARCH)/testnet/
	@zip -r ./develap-binaries/windows_$(ARCH)-v$(VERSION).zip ./develap-binaries/windows_$(ARCH)

version:
	@echo $(VERSION)