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
    privateKey = "1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99"
    fileToUpload = "test/sample.txt"

    log.Println("--> Upload and Sign")
	hive := elastosadenine.NewHive(grpcServerHost, grpcServerPort, production)
	defer hive.Close()
	response := hive.UploadAndSign(apiKeyToUse, didToUse, network, privateKey, fileToUpload)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}
