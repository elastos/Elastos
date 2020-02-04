package elastosadenine

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"github.com/cyber-republic/go-grpc-adenine/elastosadenine/stubs/wallet"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"log"
)

type Wallet struct {
	Connection *grpc.ClientConn
}

type InputWallet struct {
	Address string `json:"address"`
	Chain string	`json:"chain"`
}

func NewWallet(host string, port int, production bool) *Wallet {
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
	return &Wallet{Connection: conn}
}

func (w *Wallet) Close() {
	w.Connection.Close()
}

func (w *Wallet) CreateWallet(apiKey, network string) *wallet.Response {
	client := wallet.NewWalletClient(w.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	response, err := client.CreateWallet(ctx, &wallet.Request{
		ApiKey: apiKey,
		Network: network,
	})
	if err != nil {
		log.Fatalf("Failed to execute 'CreateWallet' method: %v", err)
	}
	return response
}

func (w *Wallet) ViewWallet(apiKey, network, chain, address string) *wallet.Response {
	client := wallet.NewWalletClient(w.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	reqData, _ := json.Marshal(InputWallet{
		Address: address,
		Chain: chain,
	})
	response, err := client.ViewWallet(ctx, &wallet.Request{
		ApiKey: apiKey,
		Network: network,
		Input: string(reqData),
	})
	if err != nil {
		log.Fatalf("Failed to execute 'ViewWallet' method: %v", err)
	}
	return response
}

func (w *Wallet) RequestELA(apiKey, chain, address string) *wallet.Response {
	client := wallet.NewWalletClient(w.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	reqData, _ := json.Marshal(InputWallet{
		Address: address,
		Chain: chain,
	})
	response, err := client.RequestELA(ctx, &wallet.Request{
		ApiKey: apiKey,
		Input: string(reqData),
	})
	if err != nil {
		log.Fatalf("Failed to execute 'RequestELA' method: %v", err)
	}
	return response
}