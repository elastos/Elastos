package elastosadenine

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"log"
	"time"

	"github.com/cyber-republic/go-grpc-adenine/v2/elastosadenine/stubs/wallet"
	"github.com/dgrijalva/jwt-go"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"google.golang.org/grpc/metadata"
)

type Wallet struct {
	Connection *grpc.ClientConn
}

type JWTInfoCreateWallet struct {
	Network string `json:"network"`
}

type JWTInfoViewWallet struct {
	Network string `json:"network"`
	Address string `json:"address"`
	Chain   string `json:"chain"`
}

type JWTInfoRequestELA struct {
	Address string `json:"address"`
	Chain   string `json:"chain"`
}

func NewWallet(host string, port int, production bool) *Wallet {
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
	return &Wallet{Connection: conn}
}

func (w *Wallet) Close() {
	w.Connection.Close()
}

func (w *Wallet) CreateWallet(apiKey, did, network string) Response {
	var outputData string
	client := wallet.NewWalletClient(w.Connection)

	jwtInfo, _ := json.Marshal(JWTInfoCreateWallet{
		Network: network,
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
		log.Fatalf("Failed to execute 'CreateWallet' method: %v", err)
	}
	md := metadata.Pairs("did", did)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	ctx = metadata.NewOutgoingContext(ctx, md)

	response, err := client.CreateWallet(ctx, &wallet.Request{Input: jwtTokenString})
	if err != nil {
		log.Fatalf("Failed to execute 'CreateWallet' method: %v", err)
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
			log.Fatalf("Failed to execute 'CreateWallet' method: %v", err)
		}
	}
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}

func (w *Wallet) RequestELA(apiKey, did, chain, address string) Response {
	var outputData string
	client := wallet.NewWalletClient(w.Connection)

	jwtInfo, _ := json.Marshal(JWTInfoRequestELA{
		Address: address,
		Chain:   chain,
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
		log.Fatalf("Failed to execute 'RequestELA' method: %v", err)
	}
	md := metadata.Pairs("did", did)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	ctx = metadata.NewOutgoingContext(ctx, md)

	response, err := client.RequestELA(ctx, &wallet.Request{Input: jwtTokenString})
	if err != nil {
		log.Fatalf("Failed to execute 'RequestELA' method: %v", err)
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
			log.Fatalf("Failed to execute 'RequestELA' method: %v", err)
		}
	}
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}
