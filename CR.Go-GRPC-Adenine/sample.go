package main

import (
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
	apiKeyToUse := "XDOoHVrENHOxlzQdt7rO3f3uldYTBpwXGu7xgJiA5GqcCWgaPVQA8XeYyC8JFi1Z"
	privateKeyToUse := "1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99"

	healthCheckTest(grpcServerHost, grpcServerPort, production)

	if *service == "generate_api_key" {
		generateAPIKeyDemo(grpcServerHost, grpcServerPort, production, mnemonicToUse, didToUse)
	} else if *service == "get_api_key" {
		getAPIKeyDemo(grpcServerHost, grpcServerPort, production, mnemonicToUse, didToUse)
	} else if *service == "upload_and_sign" {
		uploadAndSignDemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, didToUse, network, privateKeyToUse)
	} else if *service == "verify_and_show" {
		verifyAndShowDemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, didToUse, network, privateKeyToUse)
	} else if *service == "create_wallet" {
		createWalletDemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, didToUse, network)
	} else if *service == "view_wallet" {
		viewWalletDemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, didToUse, network)
	} else if *service == "request_ela" {
		requestELADemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, didToUse)
	} else if *service == "deploy_eth_contract" {
		deployETHContractDemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, didToUse, network)
	} else if *service == "watch_eth_contract" {
		watchETHContractDemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, didToUse, network)
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
		log.Printf("Output: %s", responseSSAdenine.Output)
	} else {
		log.Printf("Error Message: %s", responseSSAdenine.StatusMessage)
	}

	log.Println("--> Generate API Key - MNEMONICS")
	responseMnemonics := common.GenerateAPIRequestMnemonic(mnemonicToUse, didToUse)
	if responseMnemonics.Status {
		log.Printf("Output: %s", responseMnemonics.Output)
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
		log.Printf("Output: %s", responseSSAdenine.Output)
	} else {
		log.Printf("Error Message: %s", responseSSAdenine.StatusMessage)
	}

	log.Println("--> Get API Key - MNEMONICS")
	responseMnemonics := common.GetAPIKeyMnemonic(mnemonicToUse, didToUse)
	if responseMnemonics.Status {
		log.Printf("Output: %s", responseMnemonics.Output)
	} else {
		log.Printf("Error Message: %s", responseMnemonics.StatusMessage)
	}
}

func uploadAndSignDemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, didToUse, network, privateKeyToUse string) {
	log.Println("--> Upload and Sign")
	hive := elastosadenine.NewHive(grpcServerHost, grpcServerPort, production)
	defer hive.Close()
	response := hive.UploadAndSign(apiKeyToUse, didToUse, network, privateKeyToUse, "test/sample.txt")
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}

func verifyAndShowDemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, didToUse, network, privateKeyToUse string) {
	log.Println("--> Verify and Show")
	hive := elastosadenine.NewHive(grpcServerHost, grpcServerPort, production)
	defer hive.Close()
	response := hive.VerifyAndShow(apiKeyToUse, didToUse, network, privateKeyToUse, "516D594354423844666433393575693568644A5874444D5A576D524553796B5A4A5838733235613962694A4C4853",
                						"022316EB57646B0444CB97BE166FBE66454EB00631422E03893EE49143B4718AB8",
                						"CABB589C979B19C5FB3BB544C743024A0F712BD361C6E4F2420FC2C56238D06011A91C446C6B25CF3ED588BAFD2C1B622A0363F50D94AC95401482DC245D8571",
                						"QmYCTB8Dfd395ui5hdJXtDMZWmRESykZJX8s25a9biJLHS")
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
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

func createWalletDemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse string, didToUse string, network string) {
	log.Println("--> Create Wallet")
	wallet := elastosadenine.NewWallet(grpcServerHost, grpcServerPort, production)
	defer wallet.Close()
	response := wallet.CreateWallet(apiKeyToUse, didToUse, network)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}

func viewWalletDemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, didToUse, network string) {
	log.Println("--> View Wallet")
	wallet := elastosadenine.NewWallet(grpcServerHost, grpcServerPort, production)
	defer wallet.Close()
	var (
		address = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
		addressEth = "0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e"
	)
	// Mainchain
	response := wallet.ViewWallet(apiKeyToUse, didToUse, network, "mainchain", address)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}

	// DID Sidechain
	response = wallet.ViewWallet(apiKeyToUse, didToUse, network, "did", address)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}

	// Token Sidechain
	response = wallet.ViewWallet(apiKeyToUse, didToUse, network, "token", address)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}

	// Eth Sidechain
	response = wallet.ViewWallet(apiKeyToUse, didToUse, network, "eth", addressEth)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}

func requestELADemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, didToUse string) {
	log.Println("\n--> Request ELA")
	wallet := elastosadenine.NewWallet(grpcServerHost, grpcServerPort, production)
	defer wallet.Close()
	var (
		address = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
		addressEth = "0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e"
	)
	// Mainchain
	response := wallet.RequestELA(apiKeyToUse, didToUse, "mainchain", address)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}

	// DID Sidechain
	response = wallet.RequestELA(apiKeyToUse, didToUse, "did", address)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}

	// Token Sidechain
	response = wallet.RequestELA(apiKeyToUse, didToUse, "token", address)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
	
	// Eth Sidechain
	response = wallet.RequestELA(apiKeyToUse, didToUse, "eth", addressEth)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}

func deployETHContractDemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, didToUse, network string) {
	log.Println("--> Deploy ETH Contract")
	sidechainEth := elastosadenine.NewSidechainEth(grpcServerHost, grpcServerPort, production)
	defer sidechainEth.Close()
	var (
		address = "0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e"
		privateKey = "0x35a12175385b24b2f906d6027d440aac7bd31e1097311fa8e3cf21ceac7c4809"
		gas = 2000000
		fileName = "test/HelloWorld.sol"
	)
	response := sidechainEth.DeployEthContract(apiKeyToUse, didToUse, network, address, privateKey, gas, fileName)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}

func watchETHContractDemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, didToUse, network string) {
	log.Println("--> Watch ETH Contract")
	sidechainEth := elastosadenine.NewSidechainEth(grpcServerHost, grpcServerPort, production)
	defer sidechainEth.Close()
	var (
		contractAddress = "0xb185Ef1509d82dC163fB0EB727E77A07a3DEd256"
		contractName = "HelloWorld"
		contractCodeHash = "QmXYqHg8gRnDkDreZtXJgqkzmjujvrAr5n6KXexmfTGqHd"
	)
	response := sidechainEth.WatchEthContract(apiKeyToUse, didToUse, network, contractAddress, contractName, contractCodeHash)
	if response.Status {
		log.Printf("Status Message : %s", response.StatusMessage)
		log.Printf("Output: %s", response.Output)
	} else {
		log.Printf("Error Message: %s", response.StatusMessage)
	}
}