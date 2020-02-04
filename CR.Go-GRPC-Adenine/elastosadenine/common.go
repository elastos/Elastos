package elastosadenine

import (
	"context"
	"crypto/tls"
	"fmt"
	"github.com/cyber-republic/go-grpc-adenine/elastosadenine/stubs/common"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"log"
)

type Common struct {
	Connection *grpc.ClientConn
}

func NewCommon(host string, port int, production bool) *Common {
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
	return &Common{Connection: conn}
}

func (c *Common) Close() {
	c.Connection.Close()
}

func (c *Common) GenerateAPIRequestMnemonic(mnemonic string) *common.Response {
	client := common.NewCommonClient(c.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	response, err := client.GenerateAPIRequestMnemonic(ctx, &common.RequestMnemonic{
		Mnemonic: mnemonic,
	})
	if err != nil {
		log.Fatalf("Failed to execute 'GenerateAPIRequestMnemonic' method: %v", err)
	}
	return response
}

func (c *Common) GenerateAPIRequest(secretKey, did string) *common.Response {
	client := common.NewCommonClient(c.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	response, err := client.GenerateAPIRequest(ctx, &common.Request{SecretKey: secretKey, Did: did})
	if err != nil {
		log.Fatalf("Failed to execute 'GenerateAPIRequest' method: %v", err)
	}
	return response
}

func (c *Common) GetAPIKeyMnemonic(mnemonic string) *common.Response {
	client := common.NewCommonClient(c.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	response, err := client.GetAPIKeyMnemonic(ctx, &common.RequestMnemonic{Mnemonic: mnemonic})
	if err != nil {
		log.Fatalf("Failed to execute 'GetAPIKeyMnemonic' method: %v", err)
	}
	return response
}

func (c *Common) GetAPIKey(secretKey, did string) *common.Response {
	client := common.NewCommonClient(c.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	response, err := client.GetAPIKey(ctx, &common.Request{SecretKey: secretKey, Did: did})
	if err != nil {
		log.Fatalf("Failed to execute 'GetAPIKey' method: %v", err)
	}
	return response
}
