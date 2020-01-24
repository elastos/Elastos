package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"strconv"

	"github.com/joho/godotenv"

	"github.com/cyber-republic/go-grpc-adenine/elastosadenine"
	"github.com/cyber-republic/go-grpc-adenine/elastosadenine/stubs/health_check"
)

func main() {
	flag.Usage = func() {
		helpMessage := `usage: go run sample.go [-h] [-s SERVICE]

sample.go 
						
optional arguments:
  -h, --help  Types of services supported: generate_api_key, get_api_key,
		upload_and_sign, verify_and_show, create_wallet, view_wallet,
		request_ela, deploy_eth_contract, watch_eth_contract
-s SERVICE
`
		fmt.Printf(helpMessage)
	}
	service := flag.String("s", "", "Type of service")
	flag.Parse()

	err := godotenv.Load()
	if err != nil {
		log.Fatal("Error loading .env file")
	}
	grpcServerHost := os.Getenv("GRPC_SERVER_HOST")
	grpcServerPort, _ := strconv.Atoi(os.Getenv("GRPC_SERVER_PORT"))
	production, _ := strconv.ParseBool(os.Getenv("PRODUCTION"))

	network := "gmunet"
	mnemonicToUse := "obtain pill nest sample caution stone candy habit silk husband give net"
	didToUse := "n84dqvIK9O0LIPXi27uL0aRnoR45Exdxl218eQyPDD4lW8RPov"
	apiKeyToUse := "vtBX52Sr9tE4vgpgkLN9ZYOt5mw4d1d3y4JvbQNVG9VF1kJecQ9BA98bUjb4YnXO"

	healthCheckTest(grpcServerHost, grpcServerPort, production)

	if *service == "generate_api_key" {
		generateAPIKeyDemo(grpcServerHost, grpcServerPort, production, mnemonicToUse, didToUse)
	} else if *service == "get_api_key" {
		getAPIKeyDemo(grpcServerHost, grpcServerPort, production, mnemonicToUse, didToUse)
	} else if *service == "upload_and_sign" {
		uploadAndSignDemo()
	} else if *service == "verify_and_show" {
		verifyAndShowDemo()
	} else if *service == "create_wallet" {
		createWalletDemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, network)
	} else if *service == "view_wallet" {
		viewWalletDemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, network)
	} else if *service == "request_ela" {
		requestELADemo(grpcServerHost, grpcServerPort, production, apiKeyToUse)
	} else if *service == "deploy_eth_contract" {
		deployETHContractDemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, network)
	} else if *service == "watch_eth_contract" {
		watchETHContractDemo()
	}
}

func healthCheckTest(grpcServerHost string, grpcServerPort int, production bool) {
	log.Println("--> Checking the health status of grpc server")
	healthCheck := elastosadenine.NewHealthCheck(grpcServerHost, grpcServerPort, production)
	defer healthCheck.Close()
	response := healthCheck.Check()
	if response.Status != health_check.HealthCheckResponse_SERVING {
		log.Println("grpc server is not running properly")
	} else {
		log.Println("grpc server is running")
	}
}

func generateAPIKeyDemo(grpcServerHost string, grpcServerPort int, production bool, mnemonicToUse, didToUse string) {
	common := elastosadenine.NewCommon(grpcServerHost, grpcServerPort, production)
	defer common.Close()

	log.Println("--> Generate API Key - SHARED_SECRET_ADENINE")
	sharedSecretAdenine := os.Getenv("SHARED_SECRET_ADENINE")
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
	sharedSecretAdenine := os.Getenv("SHARED_SECRET_ADENINE")
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

func uploadAndSignDemo() {
	log.Println("--> Upload and Sign")
}

func verifyAndShowDemo() {
	log.Println("--> Verify and Show")
}

func createWalletDemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, network string) {
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

func viewWalletDemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, network string) {
	log.Println("--> View Wallet")
	wallet := elastosadenine.NewWallet(grpcServerHost, grpcServerPort, production)
	defer wallet.Close()
	var (
		address = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
		address_eth = "0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e"
	)
	// Mainchain
	response := wallet.ViewWallet(apiKeyToUse, network, "mainchain", address)
	if response.Output != "" {
		output := []byte(response.Output)
		var jsonOutput map[string]interface{}
		json.Unmarshal(output, &jsonOutput)
		log.Printf("Status Message : %s", response.StatusMessage)
		result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
		log.Printf(string(result))
	}
	// DID Sidechain
	response = wallet.ViewWallet(apiKeyToUse, network, "did", address)
	if response.Output != "" {
		output := []byte(response.Output)
		var jsonOutput map[string]interface{}
		json.Unmarshal(output, &jsonOutput)
		log.Printf("Status Message : %s", response.StatusMessage)
		result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
		log.Printf(string(result))
	}
	// Token Sidechain
	response = wallet.ViewWallet(apiKeyToUse, network, "token", address)
	if response.Output != "" {
		output := []byte(response.Output)
		var jsonOutput map[string]interface{}
		json.Unmarshal(output, &jsonOutput)
		log.Printf("Status Message : %s", response.StatusMessage)
		result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
		log.Printf(string(result))
	}
	// Eth Sidechain
	response = wallet.ViewWallet(apiKeyToUse, network, "eth", address_eth)
	if response.Output != "" {
		output := []byte(response.Output)
		var jsonOutput map[string]interface{}
		json.Unmarshal(output, &jsonOutput)
		log.Printf("Status Message : %s", response.StatusMessage)
		result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
		log.Printf(string(result))
	}
}

func requestELADemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse string) {
	log.Println("\n--> Request ELA")
	wallet := elastosadenine.NewWallet(grpcServerHost, grpcServerPort, production)
	defer wallet.Close()
	var (
		address = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
		address_eth = "0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e"
	)
	// Mainchain
	response := wallet.RequestELA(apiKeyToUse, "mainchain", address)
	if response.Output != "" {
		output := []byte(response.Output)
		var jsonOutput map[string]interface{}
		json.Unmarshal(output, &jsonOutput)
		log.Printf("Status Message : %s", response.StatusMessage)
		result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
		log.Printf(string(result))
	}
	// DID Sidechain
	response = wallet.RequestELA(apiKeyToUse, "did", address)
	if response.Output != "" {
		output := []byte(response.Output)
		var jsonOutput map[string]interface{}
		json.Unmarshal(output, &jsonOutput)
		log.Printf("Status Message : %s", response.StatusMessage)
		result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
		log.Printf(string(result))
	}
	// Token Sidechain
	response = wallet.RequestELA(apiKeyToUse, "token", address)
	if response.Output != "" {
		output := []byte(response.Output)
		var jsonOutput map[string]interface{}
		json.Unmarshal(output, &jsonOutput)
		log.Printf("Status Message : %s", response.StatusMessage)
		result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
		log.Printf(string(result))
	}
	// Eth Sidechain
	response = wallet.RequestELA(apiKeyToUse, "eth", address_eth)
	if response.Output != "" {
		output := []byte(response.Output)
		var jsonOutput map[string]interface{}
		json.Unmarshal(output, &jsonOutput)
		log.Printf("Status Message : %s", response.StatusMessage)
		result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
		log.Printf(string(result))
	}
}

func deployETHContractDemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, network string) {
	log.Println("--> Deploy ETH Contract")
	sidechainEth := elastosadenine.NewSidechainEth(grpcServerHost, grpcServerPort, production)
	defer sidechainEth.Close()
	var (
		address = "0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e"
		privateKey = "0x35a12175385b24b2f906d6027d440aac7bd31e1097311fa8e3cf21ceac7c4809"
		gas = 2000000
		fileName = "test/HelloWorld.sol"
	)
	response := sidechainEth.DeployEthContract(apiKeyToUse, network, address, privateKey, gas, fileName)
	if response.Output != "" {
		output := []byte(response.Output)
		var jsonOutput map[string]interface{}
		json.Unmarshal(output, &jsonOutput)
		log.Printf("Status Message : %s", response.StatusMessage)
		result, _ := json.Marshal(jsonOutput["result"].(map[string]interface{}))
		log.Printf(string(result))
	}
}

func watchETHContractDemo() {
	log.Println("--> Watch ETH Contract")
}