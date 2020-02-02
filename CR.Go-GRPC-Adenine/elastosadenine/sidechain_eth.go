package elastosadenine

import (
	"bufio"
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"strings"

	"google.golang.org/grpc"

	"github.com/cyber-republic/go-grpc-adenine/elastosadenine/stubs/sidechain_eth"
)

type SidechainEth struct {
	Connection *grpc.ClientConn
}

type InputSidechainEthDeploy struct {
	Address string `json:"eth_account_address"`
	PrivateKey string `json:"eth_private_key"`
	Gas int `json:"eth_gas"`
	ContractSource string `json:"contract_source"`
	ContractName string `json:"contract_name"`
}

type InputSidechainEthWatch struct {
	ContractAddress string `json:"contract_address"`
	ContractName string `json:"contract_name"`
	ContractCodeHash string `json:"contract_code_hash"`
}

func NewSidechainEth(host string, port int, production bool) *SidechainEth {
	address := fmt.Sprintf("%s:%d", host, port)
	opts := []grpc.DialOption{
		grpc.WithInsecure(),
	}
	conn, err := grpc.Dial(address, opts...)
	if err != nil {
		log.Fatalf("Failed to connect to gRPC server: %v", err)
	}
	return &SidechainEth{Connection: conn}
}

func (e *SidechainEth) Close() {
	e.Connection.Close()
}

func (e *SidechainEth) DeployEthContract(apiKey, network, address, privateKey string, gas int, fileName string) *sidechain_eth.Response {
	client := sidechain_eth.NewSidechainEthClient(e.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	// Read smart contract code file
	fileContentBytes, err := ioutil.ReadFile(fileName)
	if err != nil {
		log.Fatalf("Failed to read file during 'DeployEthContract' method: %v", err)
	}
	fileContentString := string(fileContentBytes)
	// Parse the solidity smart contract to get contract name
	var contractName string
	scanner := bufio.NewScanner(strings.NewReader(fileContentString))
	for scanner.Scan() {
		line := scanner.Text()
		if strings.Contains(line, "contract") {
			idx := strings.Index(line, "contract ")
			contractName = strings.Split(line[idx+9:], " ")[0]
		}
	}
	reqData, _ := json.Marshal(InputSidechainEthDeploy{
		Address: address,
		PrivateKey: privateKey,
		Gas: gas,
		ContractSource: fileContentString,
		ContractName: contractName,
	})
	response, err := client.DeployEthContract(ctx, &sidechain_eth.Request{
		ApiKey:               apiKey,
		Network:              network,
		Input:                string(reqData),
	})
	if err != nil {
		log.Fatalf("Failed to execute 'DeployEthContract' method: %v", err)
	}
	return response
}

func (e *SidechainEth) WatchEthContract(apiKey, network, contractAddress, contractName, contractCodeHash string) *sidechain_eth.Response {
	client := sidechain_eth.NewSidechainEthClient(e.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	reqData, _ := json.Marshal(InputSidechainEthWatch{
		ContractAddress:  contractAddress,
		ContractName:     contractName,
		ContractCodeHash: contractCodeHash,
	})
	response, err := client.WatchEthContract(ctx, &sidechain_eth.Request{
		ApiKey:               apiKey,
		Network:              network,
		Input:                string(reqData),
	})
	if err != nil {
		log.Fatalf("Failed to execute 'WatchEthContract' method: %v", err)
	}
	return response
}