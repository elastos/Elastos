package main

import (
	"log"

	"github.com/cyber-republic/go-grpc-adenine/elastosadenine"
)

const (
    sharedSecretAdenine string = "7XDnFBdHafpPyIC4nrtuJ5EUYVqdEKjW"
)

func main() {
    grpcServerHost := "localhost"
    grpcServerPort := 8001
    production := false
    
    did := "iHdasfhasdflkHdasfasdfD"

    generateAPIKeyDemo(grpcServerHost, grpcServerPort, production, did)
    getAPIKeyDemo(grpcServerHost, grpcServerPort, production, did)
}

func generateAPIKeyDemo(grpcServerHost string, grpcServerPort int, production bool, didToUse string) {
	common := elastosadenine.NewCommon(grpcServerHost, grpcServerPort, production)
	defer common.Close()

	log.Println("--> Generate API Key - SHARED_SECRET_ADENINE")
	responseSSAdenine := common.GenerateAPIRequest(sharedSecretAdenine, didToUse)
	if responseSSAdenine.Status {
		log.Printf("Output: %s", responseSSAdenine.Output)
	} else {
		log.Printf("Error Message: %s", responseSSAdenine.StatusMessage)
	}
}

func getAPIKeyDemo(grpcServerHost string, grpcServerPort int, production bool, didToUse string) {
	common := elastosadenine.NewCommon(grpcServerHost, grpcServerPort, production)
	defer common.Close()

	log.Println("--> Get API Key - SHARED_SECRET_ADENINE")
	responseSSAdenine := common.GetAPIKey(sharedSecretAdenine, didToUse)
	if responseSSAdenine.Status {
		log.Printf("Output: %s", responseSSAdenine.Output)
	} else {
		log.Printf("Error Message: %s", responseSSAdenine.StatusMessage)
	}
}

