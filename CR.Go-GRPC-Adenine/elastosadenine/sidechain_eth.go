package elastosadenine

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"log"
	"time"

	"github.com/antlr/antlr4/runtime/Go/antlr"
	"github.com/cyber-republic/go-grpc-adenine/v2/elastosadenine/stubs/sidechain_eth"
	"github.com/dgrijalva/jwt-go"
	"github.com/susmit/Solidity-Go-Parser/parser"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"google.golang.org/grpc/metadata"
)

type SidechainEth struct {
	Connection *grpc.ClientConn
}

type JWTInfoSidechainEthDeploy struct {
	Network        string `json:"network"`
	Address        string `json:"eth_account_address"`
	PrivateKey     string `json:"eth_private_key"`
	Gas            int    `json:"eth_gas"`
	ContractSource string `json:"contract_source"`
	ContractName   string `json:"contract_name"`
}

type JWTInfoSidechainEthWatch struct {
	Network          string `json:"network"`
	ContractAddress  string `json:"contract_address"`
	ContractName     string `json:"contract_name"`
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
			grpc.WithMaxMsgSize(grpcMaxMsgSize),
		}
	} else {
		config := &tls.Config{
			InsecureSkipVerify: true,
		}
		opts = []grpc.DialOption{
			grpc.WithTransportCredentials(credentials.NewTLS(config)),
			grpc.WithMaxMsgSize(grpcMaxMsgSize),
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

func (e *SidechainEth) DeployEthContract(apiKey, did, network, address, privateKey string, gas int, fileName string) Response {
	var outputData string
	client := sidechain_eth.NewSidechainEthClient(e.Connection)

	// Parse the solidity smart contract
	contractSource, _ := antlr.NewFileStream(fileName)
	lexer := parser.NewSolidityLexer(contractSource)
	stream := antlr.NewCommonTokenStream(lexer, 0)
	p := parser.NewSolidityParser(stream)
	solidityListener := SolidityListener{}
	antlr.ParseTreeWalkerDefault.Walk(&solidityListener, p.SourceUnit())

	jwtInfo, _ := json.Marshal(JWTInfoSidechainEthDeploy{
		Network:        network,
		Address:        address,
		PrivateKey:     privateKey,
		Gas:            gas,
		ContractSource: fmt.Sprint(contractSource),
		ContractName:   solidityListener.ContractName,
	})

	claims := JWTClaim{
		JwtInfo: string(jwtInfo),
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: time.Now().UTC().Add(tokenExpiration).Unix(),
		},
	}

	jwtToken := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	jwtTokenString, err := jwtToken.SignedString([]byte(apiKey))
	if err != nil {
		log.Fatalf("Failed to execute 'DeployEthContract' method: %v", err)
	}
	md := metadata.Pairs("did", did)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	ctx = metadata.NewOutgoingContext(ctx, md)

	response, err := client.DeployEthContract(ctx, &sidechain_eth.Request{Input: jwtTokenString})
	if err != nil {
		log.Fatalf("Failed to execute 'DeployEthContract' method: %v", err)
	}

	if response.Status == true {
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
			if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
				return nil, fmt.Errorf("unexpected signing method: %v", recvToken.Header["alg"])
			}
			return []byte(apiKey), nil
		})

		if recvClaims, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
			strMap := recvClaims["jwt_info"].(map[string]interface{})
			result, _ := json.Marshal(strMap["result"].(map[string]interface{}))
			outputData = string(result)
		} else {
			log.Fatalf("Failed to execute 'DeployEthContract' method: %v", err)
		}
	}
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}

func (e *SidechainEth) WatchEthContract(apiKey, did, network, contractAddress, contractName, contractCodeHash string) Response {
	var outputData string
	client := sidechain_eth.NewSidechainEthClient(e.Connection)

	jwtInfo, _ := json.Marshal(JWTInfoSidechainEthWatch{
		Network:          network,
		ContractAddress:  contractAddress,
		ContractName:     contractName,
		ContractCodeHash: contractCodeHash,
	})

	claims := JWTClaim{
		JwtInfo: string(jwtInfo),
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: time.Now().UTC().Add(tokenExpiration).Unix(),
		},
	}

	jwtToken := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	jwtTokenString, err := jwtToken.SignedString([]byte(apiKey))
	if err != nil {
		log.Fatalf("Failed to execute 'WatchEthContract' method: %v", err)
	}
	md := metadata.Pairs("did", did)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	ctx = metadata.NewOutgoingContext(ctx, md)

	response, err := client.WatchEthContract(ctx, &sidechain_eth.Request{Input: jwtTokenString})
	if err != nil {
		log.Fatalf("Failed to execute 'WatchEthContract' method: %v", err)
	}

	if response.Status == true {
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
			if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
				return nil, fmt.Errorf("unexpected signing method: %v", recvToken.Header["alg"])
			}
			return []byte(apiKey), nil
		})

		if recvClaims, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
			strMap := recvClaims["jwt_info"].(map[string]interface{})
			result, _ := json.Marshal(strMap["result"].(map[string]interface{}))
			outputData = string(result)
		} else {
			log.Fatalf("Failed to execute 'WatchEthContract' method: %v", err)
		}
	}
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}
