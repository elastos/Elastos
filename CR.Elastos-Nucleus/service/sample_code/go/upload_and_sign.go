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
    privateKey = "1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99"
    fileToUpload = "test/sample.txt"
    
    log.Println("--> Upload and Sign")
    hive := elastosadenine.NewHive(grpcServerHost, grpcServerPort, production)
    defer hive.Close()
    response := hive.UploadAndSign(apiKey, network, privateKey, fileToUpload)
    if response.Output != "" {
    	output := []byte(response.Output)
    	var jsonOutput map[string]interface{}
    	json.Unmarshal(output, &jsonOutput)
    	log.Printf("Status Message : %s", response.StatusMessage)
    	result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
    	log.Printf(string(result))
    }
}
