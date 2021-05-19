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
    
	log.Println("--> Watch ETH Contract")
	sidechainEth := elastosadenine.NewSidechainEth(grpcServerHost, grpcServerPort, production)
	defer sidechainEth.Close()
	var (
		contractAddress = "0x192277188DD72f6FAE972fd30381A574C9Dee16F"
		contractName = "HelloWorld"
		contractCodeHash = "QmXYqHg8gRnDkDreZtXJgqkzmjujvrAr5n6KXexmfTGqHd"
	)
	response := sidechainEth.WatchEthContract(apiKeyToUse, didToUse, network, contractAddress, contractName, contractCodeHash)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}
