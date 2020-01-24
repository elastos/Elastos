package elastosadenine

import (
	"fmt"
	"google.golang.org/grpc"
	"log"
)

type SidechainEth struct {
	Connection *grpc.ClientConn
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