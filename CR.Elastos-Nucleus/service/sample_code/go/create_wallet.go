package main

import (
	"encoding/json"
	"log"

	"github.com/cyber-republic/go-grpc-adenine/elastosadenine"
)

func main() {
    grpcServerHost := "localhost"
    grpcServerPort := "8001"
    production := false
    
    network := "gmunet"
    apiKeyToUse := "O2Fjcsk43uUFHqe7ygWbq0tTFj0W5gkiXxoyq1wHIQpJT8MkdKFW2LcJqBTr6AIf"

    log.Println("--> Create Wallet")
	wallet := elastosadenine.NewWallet(grpcServerHost, grpcServerPort, production)
	defer wallet.Close()
	response := wallet.CreateWallet(apiKeyToUse, network)
	if response.Output != "" {
		output := []byte(response.Output)
		var jsonOutput map[string]interface{}
		json.Unmarshal(output, &jsonOutput)
		log.Printf("Status Message : %s", response.StatusMessage)
		result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
		log.Printf(string(result))
	}
}
