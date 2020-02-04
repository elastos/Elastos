package elastosadenine

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"google.golang.org/grpc/credentials"
	"log"

	"github.com/antlr/antlr4/runtime/Go/antlr"
	"github.com/susmit/Solidity-Go-Parser/parser"
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

type SolidityListener struct {
	*parser.BaseSolidityListener
	ContractName string
}

func (s *SolidityListener) EnterContractDefinition(ctx *parser.ContractDefinitionContext) {
	s.ContractName = fmt.Sprint(ctx.GetChildren()[1].GetChildren()[0])
}

func NewSidechainEth(host string, port int, production bool) *SidechainEth {
	address := fmt.Sprintf("%s:%d", host, port)
	var opts []grpc.DialOption
	if production == false {
		opts = []grpc.DialOption{
			grpc.WithInsecure(),
		}
	} else {
		config := &tls.Config{
			InsecureSkipVerify: true,
		}
		opts = []grpc.DialOption{
			grpc.WithTransportCredentials(credentials.NewTLS(config)),
		}
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
	// Parse the solidity smart contract
	contractSource, _ := antlr.NewFileStream(fileName)
	lexer := parser.NewSolidityLexer(contractSource)
	stream := antlr.NewCommonTokenStream(lexer,0)
	p := parser.NewSolidityParser(stream)
	solidityListener := SolidityListener{}
	antlr.ParseTreeWalkerDefault.Walk(&solidityListener, p.SourceUnit())
	reqData, _ := json.Marshal(InputSidechainEthDeploy{
		Address: address,
		PrivateKey: privateKey,
		Gas: gas,
		ContractSource: fmt.Sprint(contractSource),
		ContractName: solidityListener.ContractName,
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