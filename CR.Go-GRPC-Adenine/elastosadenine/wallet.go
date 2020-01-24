package elastosadenine

import (
	"context"
	"encoding/json"
	"fmt"
	"github.com/cyber-republic/go-grpc-adenine/elastosadenine/stubs/wallet"
	"log"
	"google.golang.org/grpc"
)

type Wallet struct {
	Connection *grpc.ClientConn
}

type InputViewWallet struct {
	Address string `json:"address"`
	Chain string	`json:"chain"`
}

type InputRequestWallet struct {
	Address string	`json:"address"`
	Chain string	`json:"chain"`
}

func NewWallet(host string, port int, production bool) *Wallet {
	address := fmt.Sprintf("%s:%d", host, port)
	opts := []grpc.DialOption{
		grpc.WithInsecure(),
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
	reqData, _ := json.Marshal(InputViewWallet{
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
	reqData, _ := json.Marshal(InputRequestWallet{
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