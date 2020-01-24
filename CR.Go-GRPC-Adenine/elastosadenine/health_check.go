package elastosadenine

import (
	"context"
	"fmt"
	"google.golang.org/grpc"
	"log"

	health_check "github.com/cyber-republic/go-grpc-adenine/elastosadenine/stubs/health_check"
)

type HealthCheck struct {
	Connection *grpc.ClientConn
}

func NewHealthCheck(host string, port int, production bool) *HealthCheck {
	address := fmt.Sprintf("%s:%d", host, port)
	opts := []grpc.DialOption{
		grpc.WithInsecure(),
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

func (hc *HealthCheck) Check() *health_check.HealthCheckResponse {
	client := health_check.NewHealthClient(hc.Connection)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	response, err := client.Check(ctx, &health_check.HealthCheckRequest{})
	if err != nil {
		log.Fatalf("Failed to execute 'Check' method: %v", err)
	}
	return response
}
