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
    ethAddress = "0x4505b967d56f84647eb3a40f7c365f7d87a88bc3"
    ethPrivateKey = "0xf98fa0f1e6b6772077591ba9eefe68b227c59d9103477a4db3c411feec919abb"
    ethGas = 2000000
    smartContractFile = "test/HelloWorld.sol"

    log.Println("--> Deploy ETH Contract")
    sidechainEth := elastosadenine.NewSidechainEth(grpcServerHost, grpcServerPort, production)
    defer sidechainEth.Close()
    response := sidechainEth.DeployEthContract(apiKey, network, ethAddress, ethPrivateKey, ethGas, smartContractFile)
    if response.Output != "" {
        output := []byte(response.Output)
        var jsonOutput map[string]interface{}
        json.Unmarshal(output, &jsonOutput)
        log.Printf("Status Message : %s", response.StatusMessage)
        result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
        log.Printf(string(result))
    }
}