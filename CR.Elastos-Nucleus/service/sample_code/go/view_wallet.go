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
    
	log.Println("--> View Wallet")
	wallet := elastosadenine.NewWallet(grpcServerHost, grpcServerPort, production)
	defer wallet.Close()
	var (
		address = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
		addressEth = "0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e"
	)
	// Mainchain
	response := wallet.ViewWallet(apiKeyToUse, didToUse, network, "mainchain", address)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}

	// DID Sidechain
	response = wallet.ViewWallet(apiKeyToUse, didToUse, network, "did", address)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}

	// Token Sidechain
	response = wallet.ViewWallet(apiKeyToUse, didToUse, network, "token", address)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}

	// Eth Sidechain
	response = wallet.ViewWallet(apiKeyToUse, didToUse, network, "eth", addressEth)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}
