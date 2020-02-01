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
    
    apiKey = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    network = "gmunet"
    address = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
    addressEth = "0x282c2795B9722d638778f5a1A0045c60b330F1A0"
    
    log.Println("--> View Wallet")
    wallet := elastosadenine.NewWallet(grpcServerHost, grpcServerPort, production)
    defer wallet.Close()
    // Mainchain
    response := wallet.ViewWallet(apiKey, network, "mainchain", address)
    if response.Output != "" {
    	output := []byte(response.Output)
    	var jsonOutput map[string]interface{}
    	json.Unmarshal(output, &jsonOutput)
    	log.Printf("Status Message : %s", response.StatusMessage)
    	result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
    	log.Printf(string(result))
    }
    // DID Sidechain
    response = wallet.ViewWallet(apiKey, network, "did", address)
    if response.Output != "" {
    	output := []byte(response.Output)
    	var jsonOutput map[string]interface{}
    	json.Unmarshal(output, &jsonOutput)
    	log.Printf("Status Message : %s", response.StatusMessage)
    	result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
    	log.Printf(string(result))
    }
    // Token Sidechain
    response = wallet.ViewWallet(apiKey, network, "token", address)
    if response.Output != "" {
    	output := []byte(response.Output)
    	var jsonOutput map[string]interface{}
    	json.Unmarshal(output, &jsonOutput)
    	log.Printf("Status Message : %s", response.StatusMessage)
    	result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
    	log.Printf(string(result))
    }
    // Eth Sidechain
    response = wallet.ViewWallet(apiKey, network, "eth", addressEth)
    if response.Output != "" {
    	output := []byte(response.Output)
    	var jsonOutput map[string]interface{}
    	json.Unmarshal(output, &jsonOutput)
    	log.Printf("Status Message : %s", response.StatusMessage)
    	result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
    	log.Printf(string(result))
    }
}
