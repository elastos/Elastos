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
    messageHash = "516D5A554C6B43454C6D6D6B35584E664367546E437946674156784252425879444847474D566F4C464C6958454E"
    publicKey = "022316EB57646B0444CB97BE166FBE66454EB00631422E03893EE49143B4718AB8"
    signature = "15FDD2752B686AF7CABE8DF72FCCC91AC25577C6AFB70A81A1D987DAACAE298621E227D585B7020100228AEF96D22AD0403612FFAEDCD7CA3A2070455418181C"
    fileHash = "QmZULkCELmmk5XNfCgTnCyFgAVxBRBXyDHGGMVoLFLiXEN"

    log.Println("--> Verify and Show")
	hive := elastosadenine.NewHive(grpcServerHost, grpcServerPort, production)
	defer hive.Close()
	response := hive.VerifyAndShow(apiKeyToUse, didToUse, network, privateKey, messageHash, publicKey, signature, fileHash)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		downloadPath := "test/sample_from_hive.txt"
		log.Printf("Download Path : %s", downloadPath)
		// Open a new file for writing only
    	file, err := os.OpenFile(
			downloadPath,
        	os.O_WRONLY|os.O_TRUNC|os.O_CREATE,
        	0666,
    	)
    	if err != nil {
        	log.Fatal(err)
    	}
    	defer file.Close()

    	// Write bytes to file
    	byteSlice := response.Output
    	bytesWritten, err := file.Write(byteSlice)
    	if err != nil {
        	log.Fatal(err)
    	}
    	log.Printf("Wrote %d bytes.\n", bytesWritten)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}
