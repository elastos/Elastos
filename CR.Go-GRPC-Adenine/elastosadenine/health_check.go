package elastosadenine

import (
	"context"
	"crypto/tls"
	"fmt"
	"log"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"

	"github.com/cyber-republic/go-grpc-adenine/v2/elastosadenine/stubs/health_check"
)

type HealthCheck struct {
	Connection *grpc.ClientConn
}

func NewHealthCheck(host string, port int, production bool) *HealthCheck {
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
	return &HealthCheck{Connection: conn}
}

func (hc *HealthCheck) Close() {
	hc.Connection.Close()
}

func (hc *HealthCheck) Check() (*health_check.HealthCheckResponse, error) {
	client := health_check.NewHealthClient(hc.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	response, err := client.Check(ctx, &health_check.HealthCheckRequest{})
	if err != nil {
		log.Printf("Failed to execute 'Check' method: %v", err)
		return response, err 
	}
	return response, nil
}
