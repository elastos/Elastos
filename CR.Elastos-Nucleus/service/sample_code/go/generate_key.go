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
    
    did := "iHdasfhasdflkHdasfasdfD"
    mnemonic := "obtain pill nest sample caution stone candy habit silk husband give net"
    
    generateAPIKeyDemo(grpcServerHost, grpcServerPort, production, mnemonic, did)
    getAPIKeyDemo(grpcServerHost, grpcServerPort, production, mnemonic, did)
}

func generateAPIKeyDemo(grpcServerHost string, grpcServerPort int, production bool, mnemonicToUse, didToUse string) {
    common := elastosadenine.NewCommon(grpcServerHost, grpcServerPort, production)
    defer common.Close()
    
    log.Println("--> Generate API Key - SHARED_SECRET_ADENINE")
    sharedSecretAdenine := "7XDnFBdHafpPyIC4nrtuJ5EUYVqdEKjW"
    responseSSAdenine := common.GenerateAPIRequest(sharedSecretAdenine, didToUse)
    if responseSSAdenine.Status {
    	log.Printf("Api Key: %s", responseSSAdenine.ApiKey)
    } else {
    	log.Printf("Error Message: %s", responseSSAdenine.StatusMessage)
    }
    
    log.Println("--> Generate API Key - MNEMONICS")
    responseMnemonics := common.GenerateAPIRequestMnemonic(mnemonicToUse)
    if responseMnemonics.Status {
    	log.Printf("Api Key: %s", responseMnemonics.ApiKey)
    } else {
    	log.Printf("Error Message: %s", responseMnemonics.StatusMessage)
    }
}

func getAPIKeyDemo(grpcServerHost string, grpcServerPort int, production bool, mnemonicToUse, didToUse string) {
    common := elastosadenine.NewCommon(grpcServerHost, grpcServerPort, production)
    defer common.Close()
    
    log.Println("--> Get API Key - SHARED_SECRET_ADENINE")
    sharedSecretAdenine := "7XDnFBdHafpPyIC4nrtuJ5EUYVqdEKjW"
    responseSSAdenine := common.GetAPIKey(sharedSecretAdenine, didToUse)
    if responseSSAdenine.Status {
    	log.Printf("Api Key: %s", responseSSAdenine.ApiKey)
    } else {
    	log.Printf("Error Message: %s", responseSSAdenine.StatusMessage)
    }
    
    log.Println("--> Get API Key - MNEMONICS")
    responseMnemonics := common.GetAPIKeyMnemonic(mnemonicToUse)
    if responseMnemonics.Status {
    	log.Printf("Api Key: %s", responseMnemonics.ApiKey)
    } else {
    	log.Printf("Error Message: %s", responseMnemonics.StatusMessage)
    }
}
