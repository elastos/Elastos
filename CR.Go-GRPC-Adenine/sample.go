package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"strconv"

	"github.com/joho/godotenv"

	"github.com/cyber-republic/go-grpc-adenine/v2/elastosadenine"
	"github.com/cyber-republic/go-grpc-adenine/v2/elastosadenine/stubs/health_check"
)

func main() {
	flag.Usage = func() {
		helpMessage := `usage: go run sample.go [-h] [-s SERVICE]

sample.go 
						
optional arguments:
  -h, --help  Types of services supported: generate_api_key, get_api_key, 
		node_rpc_methods, upload_and_sign, verify_and_show, create_wallet, 
		view_wallet, request_ela, deploy_eth_contract, watch_eth_contract
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

	network := "mainnet"
	didToUse := "n84dqvIK9O0LIPXi27uL0aRnoR45Exdxl218eQyPDD4lW8RPov"
	apiKeyToUse := "XtkJiYGbqyJS9MwA9LLH0mz1T8FxpRYqRVIi1uyU8VEUdfM0ReaqTFJpF5OLz9wm"
	privateKeyToUse := "1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99"

	err = healthCheckTest(grpcServerHost, grpcServerPort, production)
	if err != nil {
		log.Fatalln(err)
	}

	if *service == "generate_api_key" {
		generateAPIKeyDemo(grpcServerHost, grpcServerPort, production, didToUse)
	} else if *service == "get_api_key" {
		getAPIKeyDemo(grpcServerHost, grpcServerPort, production, didToUse)
	} else if *service == "node_rpc_methods" {
		nodeRPCMethodsDemo(grpcServerHost, grpcServerPort, production, apiKeyToUse, didToUse, network)
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

func healthCheckTest(grpcServerHost string, grpcServerPort int, production bool) error {
	log.Println("--> Checking the health status of grpc server")
	healthCheck := elastosadenine.NewHealthCheck(grpcServerHost, grpcServerPort, production)
	defer healthCheck.Close()
	response, err := healthCheck.Check()
	if err != nil {
		return fmt.Errorf("Could not connect to grpc server: %v\n", err)
	} else if response.Status != health_check.HealthCheckResponse_SERVING {
		return fmt.Errorf("grpc server is not running properly\n")
	} 
	fmt.Println("grpc server is running\n")
	return nil
}

func generateAPIKeyDemo(grpcServerHost string, grpcServerPort int, production bool, didToUse string) {
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
}

func getAPIKeyDemo(grpcServerHost string, grpcServerPort int, production bool, didToUse string) {
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
}

func nodeRPCMethodsDemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, didToUse, network string) {
	var nodeRpc *elastosadenine.NodeRpc
	if result, err := elastosadenine.NewNodeRpc(grpcServerHost, grpcServerPort, production); err == nil {
		nodeRpc = result
	} else {
		log.Println("Could not create connection to gRPC server: %v", err)
		return
	}
	defer nodeRpc.Close()

	log.Println("--> Get current height")
	currentHeight := nodeRpc.GetCurrentHeight(apiKeyToUse, didToUse, network, "mainchain")
	log.Println("Current Height - mainchain: ", currentHeight)
	currentHeight = nodeRpc.GetCurrentHeight(apiKeyToUse, didToUse, network, "did")
	log.Println("Current Height - did sidechain: ", currentHeight)
	currentHeight = nodeRpc.GetCurrentHeight(apiKeyToUse, didToUse, network, "token")
	log.Println("Current Height - token sidechain: ", currentHeight)
	currentHeight = nodeRpc.GetCurrentHeight(apiKeyToUse, didToUse, network, "eth")
	log.Println("Current Height - eth sidechain: ", currentHeight)

	log.Println("--> Get current block info")
	currentBlockInfo := nodeRpc.GetCurrentBlockInfo(apiKeyToUse, didToUse, network, "mainchain")
	log.Println("Current block info - mainchain: ", currentBlockInfo)
	currentBlockInfo = nodeRpc.GetCurrentBlockInfo(apiKeyToUse, didToUse, network, "did")
	log.Println("Current block info - did sidechain: ", currentBlockInfo)
	currentBlockInfo = nodeRpc.GetCurrentBlockInfo(apiKeyToUse, didToUse, network, "token")
	log.Println("Current block info - token sidechain: ", currentBlockInfo)
	currentBlockInfo = nodeRpc.GetCurrentBlockInfo(apiKeyToUse, didToUse, network, "eth")
	log.Println("Current block info - eth sidechain: ", currentBlockInfo)

	log.Println("--> Get current mining info - mainchain")
	currentMiningInfo := nodeRpc.GetCurrentMiningInfo(apiKeyToUse, didToUse, network)
	log.Println("Current mining info: ", currentMiningInfo)
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
	response := hive.VerifyAndShow(apiKeyToUse, didToUse, network, privateKeyToUse, "516D53733454546F416F645172355671467654746371676F7768713841645632744C417A4637535472514D584438",
		"022316EB57646B0444CB97BE166FBE66454EB00631422E03893EE49143B4718AB8",
		"F97737AD88BD99B3374AB9FD750970A0A6328143765313CD197B2CF9DE01571F4C1FB98567A72A2DB2C9E6469F2A65C1331AC64AC121ECC526D7532BDF1E6DDD",
		"QmSs4TToAodQr5VqFvTtcqgowhq8AdV2tLAzF7STrQMXD8")
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
	var nodeRpc *elastosadenine.NodeRpc
	if result, err := elastosadenine.NewNodeRpc(grpcServerHost, grpcServerPort, production); err == nil {
		nodeRpc = result
	} else {
		log.Println("Could not create connection to gRPC servier: %v", err)
		return
	}
	defer nodeRpc.Close()
	var (
		address    = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
		addressEth = "0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e"
	)

	log.Println("--> Get current balance")
	currentBalance := nodeRpc.GetCurrentBalance(apiKeyToUse, didToUse, network, "mainchain", address).(string)
	log.Println("Current balance - mainchain:", currentBalance)
	currentBalance = nodeRpc.GetCurrentBalance(apiKeyToUse, didToUse, network, "did", address).(string)
	log.Println("Current balance - did sidechain:", currentBalance)
	currentBalanceToken := nodeRpc.GetCurrentBalance(apiKeyToUse, didToUse, network, "token", address).(map[string]string)
	log.Println("Current balance - token sidechain:", currentBalanceToken)
	currentBalanceEth := nodeRpc.GetCurrentBalance(apiKeyToUse, didToUse, network, "eth", addressEth).(string)
	log.Println("Current balance - eth sidechain:", currentBalanceEth)
}

func requestELADemo(grpcServerHost string, grpcServerPort int, production bool, apiKeyToUse, didToUse string) {
	log.Println("\n--> Request ELA")
	wallet := elastosadenine.NewWallet(grpcServerHost, grpcServerPort, production)
	defer wallet.Close()
	var (
		address    = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
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
		address    = "0x48F01b2f2b1a546927ee99dD03dCa37ff19cB84e"
		privateKey = "0x35a12175385b24b2f906d6027d440aac7bd31e1097311fa8e3cf21ceac7c4809"
		gas        = 2000000
		fileName   = "test/HelloWorld.sol"
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
		contractAddress  = "0x192277188DD72f6FAE972fd30381A574C9Dee16F"
		contractName     = "HelloWorld"
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
