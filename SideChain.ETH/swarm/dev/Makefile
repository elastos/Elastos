.PHONY: build cluster test

default: build

build:
	go build -o bin/swarm    github.com/elastos/Elastos.ELA.SideChain.ETH/cmd/swarm
	go build -o bin/geth     github.com/elastos/Elastos.ELA.SideChain.ETH/cmd/geth
	go build -o bin/bootnode github.com/elastos/Elastos.ELA.SideChain.ETH/cmd/bootnode

cluster: build
	scripts/boot-cluster.sh

test:
	go test -v github.com/elastos/Elastos.ELA.SideChain.ETH/swarm/...
