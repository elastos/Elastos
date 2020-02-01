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
    
    apiKey := "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    network := "gmunet"
    smartContractAddress := "0xdf29327c95b12A37089A6e230d5ce50F23237671"
    smartContractName := "HelloWorld"
    smartContractCodeHash := "QmRCn3tQem7UugGLE7tkchXudp4prqLtDhMRs828mUED34"
    
    log.Println("--> Watch ETH Contract")
    sidechainEth := elastosadenine.NewSidechainEth(grpcServerHost, grpcServerPort, production)
    defer sidechainEth.Close()
    response := sidechainEth.WatchEthContract(apiKey, network, smartContractAddress, smartContractName, smartContractCodeHash)
    if response.Output != "" {
    	output := []byte(response.Output)
    	var jsonOutput map[string]interface{}
    	json.Unmarshal(output, &jsonOutput)
    	log.Printf("Status Message : %s", response.StatusMessage)
    	result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
    	log.Printf(string(result))
    }
}
