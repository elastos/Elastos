package elastosadenine

import (
	"context"
	"crypto/tls"
	"fmt"
	"github.com/cyber-republic/go-grpc-adenine/elastosadenine/stubs/common"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"github.com/dgrijalva/jwt-go"
	"google.golang.org/grpc/metadata"
	"log"
	"time"
	"os"
	"encoding/json"
)

type Common struct {
	Connection *grpc.ClientConn
}

type GenerateAPIRequest struct {
    Pass string `json:"pass"`
    jwt.StandardClaims
}

type GenerateAPIRequestMnemonic struct {
    Mnemonic string `json:"mnemonic"`
}

type SendJWT struct {
    Jwtinfo string `json:"jwt_info"`
    jwt.StandardClaims
}

type Response struct {
	Output string 
	Status bool
	StatusMessage string
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

func (c *Common) GenerateAPIRequestMnemonic(mnemonic, did string) Response {
	var outputData string
	secretKey := os.Getenv("SHARED_SECRET_ADENINE")
	client := common.NewCommonClient(c.Connection)
	expirationTime := time.Now().UTC().Add(time.Minute * 12)
	
	jwtInfo, _ := json.Marshal(GenerateAPIRequestMnemonic{
		Mnemonic: mnemonic,
	})

    claims := SendJWT{
		Jwtinfo: string(jwtInfo),
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: expirationTime.Unix(),
			Issuer:    "go-grpc-adenine",
		},
	}
	
	sendToken := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	signedToken, err := sendToken.SignedString([]byte(secretKey))
	md := metadata.Pairs("did", did)
	ctx := metadata.NewOutgoingContext(context.Background(), md)
	
	response, err := client.GenerateAPIRequestMnemonic(ctx, &common.Request{Input: signedToken})
	if err != nil {
		log.Fatalf("Failed to execute 'GenerateAPIRequestMnemonic' method: %v", err)
	}
	
	if response.Status == true{
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
		    if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
		        return nil, fmt.Errorf("Unexpected signing method: %v", recvToken.Header["alg"])
		    }
		    return []byte(secretKey), nil
		})

		if recvClaims, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
		    strMap := recvClaims["jwt_info"].(map[string]interface{})
			result, _ := json.Marshal(strMap["result"].(map[string]interface{}))
			outputData = string(result)
		} else {
		    fmt.Println(err)
		}
	} 
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}

func (c *Common) GenerateAPIRequest(secretKey, did string) Response {
	var outputData string
	client := common.NewCommonClient(c.Connection)
	expirationTime := time.Now().UTC().Add(time.Minute * 12)
	
	jwtInfo, _ := json.Marshal(GenerateAPIRequest{
		Pass: "pass",
	})

    claims := SendJWT{
		Jwtinfo: string(jwtInfo),
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: expirationTime.Unix(),
			Issuer:    "go-grpc-adenine",
		},
	}
	
	sendToken := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	signedToken, err := sendToken.SignedString([]byte(secretKey))
	md := metadata.Pairs("did", did)
	ctx := metadata.NewOutgoingContext(context.Background(), md)
	
	response, err := client.GenerateAPIRequest(ctx, &common.Request{Input: signedToken})
	if err != nil {
		log.Fatalf("Failed to execute 'GenerateAPIRequest' method: %v", err)
	}
	
	if response.Status == true{
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
		    if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
		        return nil, fmt.Errorf("Unexpected signing method: %v", recvToken.Header["alg"])
		    }
		    return []byte(secretKey), nil
		})

		if recvClaims, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
		    strMap := recvClaims["jwt_info"].(map[string]interface{})
			result, _ := json.Marshal(strMap["result"].(map[string]interface{}))
			outputData = string(result)
		} else {
		    fmt.Println(err)
		}
	} 
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}

func (c *Common) GetAPIKeyMnemonic(mnemonic, did string) Response {
	var outputData string
	secretKey := os.Getenv("SHARED_SECRET_ADENINE")
	client := common.NewCommonClient(c.Connection)
	expirationTime := time.Now().UTC().Add(time.Minute * 12)
	
	jwtInfo, _ := json.Marshal(GenerateAPIRequestMnemonic{
		Mnemonic: mnemonic,
	})

    claims := SendJWT{
		Jwtinfo: string(jwtInfo),
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: expirationTime.Unix(),
			Issuer:    "go-grpc-adenine",
		},
	}
	
	sendToken := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	signedToken, err := sendToken.SignedString([]byte(secretKey))
	md := metadata.Pairs("did", did)
	ctx := metadata.NewOutgoingContext(context.Background(), md)
	
	response, err := client.GetAPIKeyMnemonic(ctx, &common.Request{Input: signedToken})
	if err != nil {
		log.Fatalf("Failed to execute 'GetAPIKeyMnemonic' method: %v", err)
	}
	
	if response.Status == true{
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
		    if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
		        return nil, fmt.Errorf("Unexpected signing method: %v", recvToken.Header["alg"])
		    }
		    return []byte(secretKey), nil
		})

		if recvClaims, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
		    strMap := recvClaims["jwt_info"].(map[string]interface{})
			result, _ := json.Marshal(strMap["result"].(map[string]interface{}))
			outputData = string(result)
		} else {
		    fmt.Println(err)
		}
	} 
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}

func (c *Common) GetAPIKey(secretKey, did string) Response {
	var outputData string
	client := common.NewCommonClient(c.Connection)
	expirationTime := time.Now().UTC().Add(time.Minute * 12)
	
	jwtInfo, _ := json.Marshal(GenerateAPIRequest{
		Pass: "pass",
	})

    claims := SendJWT{
		Jwtinfo: string(jwtInfo),
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: expirationTime.Unix(),
			Issuer:    "go-grpc-adenine",
		},
	}
	
	sendToken := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	signedToken, err := sendToken.SignedString([]byte(secretKey))
	md := metadata.Pairs("did", did)
	ctx := metadata.NewOutgoingContext(context.Background(), md)
	
	response, err := client.GetAPIKey(ctx, &common.Request{Input: signedToken})
	if err != nil {
		log.Fatalf("Failed to execute 'GetAPIKey' method: %v", err)
	}
	
	if response.Status == true{
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
		    if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
		        return nil, fmt.Errorf("Unexpected signing method: %v", recvToken.Header["alg"])
		    }
		    return []byte(secretKey), nil
		})

		if recvClaims, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
		    strMap := recvClaims["jwt_info"].(map[string]interface{})
			result, _ := json.Marshal(strMap["result"].(map[string]interface{}))
			outputData = string(result)
		} else {
		    fmt.Println(err)
		}
	} 
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}
