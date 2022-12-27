package main

import (
	"log"

	"github.com/cyber-republic/go-grpc-adenine/elastosadenine"
)

func main() {
    grpcServerHost := "localhost"
    grpcServerPort := 8001
    production := false

    network := "gmunet"
    apiKeyToUse := "O2Fjcsk43uUFHqe7ygWbq0tTFj0W5gkiXxoyq1wHIQpJT8MkdKFW2LcJqBTr6AIf"
    didToUse := "iHdasfhasdflkHdasfasdfD"

	log.Println("--> Deploy ETH Contract")
	sidechainEth := elastosadenine.NewSidechainEth(grpcServerHost, grpcServerPort, production)
	defer sidechainEth.Close()
	var (
		address = "0x4505b967d56f84647eb3a40f7c365f7d87a88bc3"
		privateKey = "0xf98fa0f1e6b6772077591ba9eefe68b227c59d9103477a4db3c411feec919abb"
		gas = 2000000
		fileName = "test/HelloWorld.sol"
	)
	response := sidechainEth.DeployEthContract(apiKeyToUse, didToUse, network, address, privateKey, gas, fileName)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}