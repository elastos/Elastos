package elastosadenine

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"log"
	"time"

	"github.com/cyber-republic/go-grpc-adenine/v2/elastosadenine/stubs/common"
	"github.com/dgrijalva/jwt-go"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"google.golang.org/grpc/metadata"
)

type Common struct {
	Connection *grpc.ClientConn
}

type JWTInfoCommon struct {
	jwt.StandardClaims
}

func NewCommon(host string, port int, production bool) *Common {
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
	return &Common{Connection: conn}
}

func (c *Common) Close() {
	c.Connection.Close()
}

func (c *Common) GenerateAPIRequest(secretKey, did string) Response {
	var outputData string
	client := common.NewCommonClient(c.Connection)

	jwtInfo, _ := json.Marshal(JWTInfoCommon{})

	claims := JWTClaim{
		JwtInfo: string(jwtInfo),
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: time.Now().UTC().Add(tokenExpiration).Unix(),
		},
	}

	jwtToken := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	jwtTokenString, err := jwtToken.SignedString([]byte(secretKey))
	if err != nil {
		log.Fatalf("Failed to execute 'GenerateAPIRequest' method: %v", err)
	}
	md := metadata.Pairs("did", did)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	ctx = metadata.NewOutgoingContext(ctx, md)

	response, err := client.GenerateAPIRequest(ctx, &common.Request{Input: jwtTokenString})
	if err != nil {
		log.Fatalf("Failed to execute 'GenerateAPIRequest' method: %v", err)
	}

	if response.Status == true {
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
			if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
				return nil, fmt.Errorf("unexpected signing method: %v", recvToken.Header["alg"])
			}
			return []byte(secretKey), nil
		})

		if recvClaims, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
			strMap := recvClaims["jwt_info"].(map[string]interface{})
			result, _ := json.Marshal(strMap["result"].(map[string]interface{}))
			outputData = string(result)
		} else {
			log.Fatalf("Failed to execute 'GenerateAPIRequest' method: %v", err)
		}
	}
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}

func (c *Common) GetAPIKey(secretKey, did string) Response {
	var outputData string
	client := common.NewCommonClient(c.Connection)

	jwtInfo, _ := json.Marshal(JWTInfoCommon{})

	claims := JWTClaim{
		JwtInfo: string(jwtInfo),
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: time.Now().UTC().Add(tokenExpiration).Unix(),
		},
	}

	jwtToken := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	jwtTokenString, err := jwtToken.SignedString([]byte(secretKey))
	if err != nil {
		log.Fatalf("Failed to execute 'GetAPIKey' method: %v", err)
	}
	md := metadata.Pairs("did", did)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	ctx = metadata.NewOutgoingContext(ctx, md)

	response, err := client.GetAPIKey(ctx, &common.Request{Input: jwtTokenString})
	if err != nil {
		log.Fatalf("Failed to execute 'GetAPIKey' method: %v", err)
	}

	if response.Status == true {
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
			if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
				return nil, fmt.Errorf("unexpected signing method: %v", recvToken.Header["alg"])
			}
			return []byte(secretKey), nil
		})

		if recvClaims, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
			strMap := recvClaims["jwt_info"].(map[string]interface{})
			result, _ := json.Marshal(strMap["result"].(map[string]interface{}))
			outputData = string(result)
		} else {
			log.Fatalf("Failed to execute 'GetAPIKey' method: %v", err)
		}
	}
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}
