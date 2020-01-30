package elastosadenine

import (
	"context"
	"encoding/json"
	"fmt"
	"github.com/cyber-republic/go-grpc-adenine/elastosadenine/stubs/hive"
	"google.golang.org/grpc"
	"log"
	"os"
	"io/ioutil"
)

type Hive struct {
	Connection *grpc.ClientConn
}

type InputUploadAndSign struct {
	PrivateKey string `json:"privateKey"`
}

type InputVerifyAndShow struct {
	Msg string `json:"msg"`
	Pub string `json:"pub"`
	Sig string `json:"sig"`
	Hash string `json:"hash"`
	PrivateKey string `json:"privateKey"`
}

func NewHive(host string, port int, production bool) *Hive {
	address := fmt.Sprintf("%s:%d", host, port)
	opts := []grpc.DialOption{
		grpc.WithInsecure(),
	}
	conn, err := grpc.Dial(address, opts...)
	if err != nil {
		log.Fatalf("Failed to connect to gRPC server: %v", err)
	}
	return &Hive{Connection: conn}
}

func (h *Hive) Close() {
	h.Connection.Close()
}

func (h *Hive) UploadAndSign(apiKey, network, privateKey, filename string) *hive.Response {
	client := hive.NewHiveClient(h.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	//converting to JSON
	reqData, _ := json.Marshal(InputUploadAndSign{
		PrivateKey: privateKey,
	})
	// Open file for reading
    file, err := os.Open(filename)
    if err != nil {
        log.Fatal(err)
    }
    data, err := ioutil.ReadAll(file)
    if err != nil {
        log.Fatal(err)
    }
    //sending the request
	response, err := client.UploadAndSign(ctx, &hive.Request{
		ApiKey: apiKey,
		Network: network,
		Input: string(reqData),
		FileContent: data,
	})
	if err != nil {
		log.Fatalf("Failed to execute 'UploadAndSign' method: %v", err)
	}
	return response
}

func (h *Hive) VerifyAndShow(apiKey, network, privateKey, msg, pub, sig, hash string) *hive.Response {
	client := hive.NewHiveClient(h.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	//converting to JSON
	reqData, _ := json.Marshal(InputVerifyAndShow{
		Msg: msg,
		Pub: pub,
		Sig: sig,
		Hash: hash,
		PrivateKey: privateKey,
	})
	response, err := client.VerifyAndShow(ctx, &hive.Request{
		ApiKey: apiKey,
		Network: network,
		Input: string(reqData),
	})
	if err != nil {
		log.Fatalf("Failed to execute 'VerifyAndShow' method: %v", err)
	}
	return response
}
