package elastosadenine

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"time"

	"github.com/cyber-republic/go-grpc-adenine/v2/elastosadenine/stubs/hive"
	"github.com/dgrijalva/jwt-go"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"google.golang.org/grpc/metadata"
)

type Hive struct {
	Connection *grpc.ClientConn
}

type JWTInfoHiveUploadAndSign struct {
	Network     string `json:"network"`
	PrivateKey  string `json:"privateKey"`
	FileContent string `json:"file_content"`
}

type JWTInfoVerifyAndShow struct {
	Network    string `json:"network"`
	Msg        string `json:"msg"`
	Pub        string `json:"pub"`
	Sig        string `json:"sig"`
	Hash       string `json:"hash"`
	PrivateKey string `json:"privateKey"`
}

func NewHive(host string, port int, production bool) *Hive {
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
	return &Hive{Connection: conn}
}

func (h *Hive) Close() {
	h.Connection.Close()
}

func (h *Hive) UploadAndSign(apiKey, did, network, privateKey, filename string) Response {
	var outputData string
	client := hive.NewHiveClient(h.Connection)

	// Open file for reading
	file, err := os.Open(filename)
	if err != nil {
		log.Fatal(err)
	}
	fileContent, err := ioutil.ReadAll(file)
	if err != nil {
		log.Fatal(err)
	}

	jwtInfo, _ := json.Marshal(JWTInfoHiveUploadAndSign{
		Network:    network,
		PrivateKey: privateKey,
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
		log.Fatalf("Failed to execute 'UploadAndSign' method: %v", err)
	}
	md := metadata.Pairs("did", did)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	ctx = metadata.NewOutgoingContext(ctx, md)

	response, err := client.UploadAndSign(ctx, &hive.Request{Input: jwtTokenString, FileContent: fileContent})
	if err != nil {
		log.Fatalf("Failed to execute 'UploadAndSign' method: %v", err)
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
			log.Fatalf("Failed to execute 'UploadAndSign' method: %v", err)
		}
	}
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}

func (h *Hive) VerifyAndShow(apiKey, did, network, privateKey, msg, pub, sig, hash string) ResponseData {
	var fileContent []byte
	client := hive.NewHiveClient(h.Connection)

	jwtInfo, _ := json.Marshal(JWTInfoVerifyAndShow{
		Network:    network,
		Msg:        msg,
		Pub:        pub,
		Sig:        sig,
		Hash:       hash,
		PrivateKey: privateKey,
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
		log.Fatalf("Failed to execute 'VerifyAndShow' method: %v", err)
	}
	md := metadata.Pairs("did", did)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	ctx = metadata.NewOutgoingContext(ctx, md)

	response, err := client.VerifyAndShow(ctx, &hive.Request{Input: jwtTokenString})
	if err != nil {
		log.Fatalf("Failed to execute 'VerifyAndShow' method: %v", err)
	}

	if response.Status == true {
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
			if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
				return nil, fmt.Errorf("unexpected signing method: %v", recvToken.Header["alg"])
			}
			return []byte(apiKey), nil
		})

		if _, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
			fileContent = response.FileContent
		} else {
			log.Fatalf("Failed to execute 'VerifyAndShow' method: %v", err)
		}
	}
	responseData := ResponseData{fileContent, response.Status, response.StatusMessage}
	return responseData
}
